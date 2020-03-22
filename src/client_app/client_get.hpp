#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include <asio.hpp>

#include "file_io.hpp"
#include "make_unique.hpp"
#include "netascii_io.hpp"
#include "packet.hpp"
#include "packet_builder.hpp"
#include "packet_parser.hpp"
#include "string_utils.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class client_get : public std::enable_shared_from_this<client_get>
{
public:
    client_get(asio::io_context& io_context, const std::string& host, std::uint16_t port,
        const std::string& remote_filename, const std::string& local_path, const std::string& mode)
        : m_io_context(io_context)
        , m_resolver(io_context)
        , m_socket(io_context)
        , m_send_timeout_timer(io_context)
        , m_host(host)
        , m_port(port)
        , m_remote_filename(remote_filename)
        , m_local_path(local_path)
        , m_mode(mode)
        , m_request_complete(false)
        , m_last_acked_packet_id(0)
        , m_response_expected(false)
        , m_retry_counter(0)
    {
        // noop
    }

    void start()
    {
        asio::ip::udp::endpoint connection_endpoint(asio::ip::address_v4::any(), 0);

        m_socket.open(connection_endpoint.protocol());
        m_socket.set_option(asio::socket_base::reuse_address(true));
        m_socket.bind(connection_endpoint);

        m_resolver.async_resolve(m_host, std::to_string(m_port),
            std::bind(&client_get::on_resolve_query, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void stop()
    {
        m_resolver.cancel();

        m_send_timeout_timer.cancel();

        asio::error_code ec;
        if (m_socket.is_open())
        {
            m_socket.close(ec);
            if (ec)
            {
                std::cerr << "Socket close failed: " << ec << std::endl;
            }
        }
    }

private:
    void on_resolve_query(const asio::error_code& ec, asio::ip::udp::resolver::results_type results)
    {
        if (ec == asio::error::operation_aborted)
        {
            // ignore, cancelled
            return;
        }

        for (const auto& x : results)
        {
            std::cout << "X: " << x.endpoint() << std::endl;
        }

        if (results.size() == 0)
        {
            std::cerr << "Cannot resolve address: " << m_host << ':' << m_port << std::endl;
            return;
        }

        m_server_endpoint = *results.cbegin();

        send_request();
    }

    void send_request()
    {
        packet_file_req packet;
        packet.m_op = OP_RRQ;
        packet.m_filename = m_remote_filename;
        packet.m_mode = m_mode;

        send_packet(packet, true, DEFAULT_RETRY_COUNTER);
    }

    template <class packet_T>
    void send_packet(const packet_T& packet, bool response_expected, int retry_counter)
    {
        m_out_packet_data = packet_builder::build_packet(packet);
        m_response_expected = response_expected;
        m_retry_counter = retry_counter;

        send_prepared_packet();
    }

    void send_prepared_packet()
    {
        m_socket.async_send_to(asio::const_buffer(m_out_packet_data.data(), m_out_packet_data.size()),
            m_server_endpoint,
            std::bind(&client_get::on_packet_sent, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void on_packet_sent(const asio::error_code& ec, std::size_t bytes_transferred)
    {
        if (ec == asio::error::operation_aborted)
        {
            // ignore, cancelled
            return;
        }

        if (ec)
        {
            std::cerr << "Packet send failed: " << ec << std::endl;
            terminate(false);
            return;
        }

        std::cout << "Packet sent: " << bytes_transferred << std::endl;

        if (m_response_expected)
        {
            m_send_timeout_timer.expires_after(std::chrono::seconds(DEFAULT_RETRY_TIMEOUT_SEC));
            m_send_timeout_timer.async_wait(
                std::bind(&client_get::on_send_timeout, shared_from_this(), std::placeholders::_1));
            request_next_receive();
        }
        else
        {
            terminate(false);
        }
    }

    void request_next_receive()
    {
        m_socket.async_receive_from(asio::buffer(m_in_packet_data), m_in_packet_endpoint,
            std::bind(
                &client_get::on_packet_received, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void on_packet_received(const asio::error_code& ec, std::size_t bytes_received)
    {
        if (ec == asio::error::operation_aborted)
        {
            // ignore, cancelled
            return;
        }

        if (ec)
        {
            std::cerr << "Error occurred: " << ec << std::endl;
            terminate(false);
            return;
        }

        std::cout << "Packet received: " << bytes_received << std::endl;

        if (m_request_complete)
        {
            if (m_server_endpoint != m_in_packet_endpoint)
            {
                std::cerr << "Received packet from unexpected source: " << m_in_packet_endpoint << std::endl;
                return;
            }
        }

        auto packet = packet_parser::parse_packet(asio::const_buffer(m_in_packet_data.data(), bytes_received));
        if (!packet)
        {
            std::cerr << "Invalid packet received of size: " << bytes_received << std::endl;
            return;
        }

        switch (packet->m_op)
        {
        case OP_DATA:
            process_data_received(std::static_pointer_cast<packet_data>(packet));
            return;

        case OP_ERROR:
            process_error_received(std::static_pointer_cast<packet_error>(packet));
            return;

        default:
            std::cerr << "Unexpected packet type: " << packet->m_op << std::endl;
            return;
        }
    }

    std::unique_ptr<writer> open_writer()
    {
        if (equal_ignore_case(m_mode, "octet"))
        {
            return stdext::make_unique<file_writer>(m_local_path);
        }
        if (equal_ignore_case(m_mode, "netascii"))
        {
            return stdext::make_unique<netascii_writer>(stdext::make_unique<file_writer>(m_local_path));
        }
        return nullptr;
    }

    void process_data_received(std::shared_ptr<packet_data> received_packet)
    {
        if (m_last_acked_packet_id + 1 != received_packet->m_block_no)
        {
            std::cerr << "Unexpected block no: " << received_packet->m_block_no << std::endl;
            return;
        }

        std::cout << "Received packet: " << received_packet->m_block_no
                  << " with bytes: " << received_packet->m_data.size() << std::endl;

        if (!m_request_complete)
        {
            m_request_complete = true;
            m_server_endpoint = m_in_packet_endpoint;

            m_writer = open_writer();
            if (!m_writer)
            {
                packet_error packet;
                packet.m_op = OP_ERROR;
                packet.m_error_code = ERRCODE_UNDEFINED;
                packet.m_error_message = "cannot open file for writing";

                send_packet(packet, false, 0);
                return;
            }
            if (!m_writer->is_open())
            {
                packet_error packet;
                packet.m_op = OP_ERROR;
                packet.m_error_code = ERRCODE_ACCESS_VIOLATION;
                packet.m_error_message = "cannot open file for writing";

                send_packet(packet, false, 0);
                return;
            }
        }

        if (!m_writer->write(received_packet->m_data.data(), received_packet->m_data.size()))
        {
            packet_error packet;
            packet.m_op = OP_ERROR;
            packet.m_error_code = ERRCODE_DISK_FULL;
            packet.m_error_message = "cannot open file for writing";

            send_packet(packet, false, 0);
            return;
        }

        packet_ack packet;
        packet.m_op = OP_ACK;
        packet.m_block_no = received_packet->m_block_no;

        bool last_packet = (received_packet->m_data.size() < DEFAULT_DATA_SIZE);

        m_last_acked_packet_id = received_packet->m_block_no;
        send_packet(packet, !last_packet, DEFAULT_RETRY_COUNTER);
    }

    void process_error_received(std::shared_ptr<packet_error> packet)
    {
        std::cout << "ERROR received: " << packet->m_error_code << ' ' << packet->m_error_message << std::endl;
        terminate(false);
    }

    void on_send_timeout(const asio::error_code& ec)
    {
        if (ec == asio::error::operation_aborted)
        {
            // ignore, cancelled
            return;
        }

        if (ec)
        {
            std::cerr << "Timer error: " << ec << std::endl;
            return;
        }

        if (--m_retry_counter > 0)
        {
            send_prepared_packet();
        }
        else
        {
            std::cerr << "No more retries" << std::endl;
            terminate(false);
        }
    }

    void terminate(bool success)
    {
        // TODO:
        stop();
    }

    asio::io_context& m_io_context;
    asio::ip::udp::resolver m_resolver;
    asio::ip::udp::socket m_socket;
    asio::system_timer m_send_timeout_timer;

    const std::string m_host;
    const std::uint16_t m_port;
    const std::string m_remote_filename;
    const std::string m_local_path;
    const std::string m_mode;

    asio::ip::udp::endpoint m_server_endpoint;

    bool m_request_complete;
    std::uint16_t m_last_acked_packet_id;

    bool m_response_expected;
    int m_retry_counter;

    std::unique_ptr<writer> m_writer;

    std::vector<std::uint8_t> m_out_packet_data;

    std::array<std::uint8_t, MAX_RECV_PACKET_SIZE> m_in_packet_data;
    asio::ip::udp::endpoint m_in_packet_endpoint;
};

} // namespace tftp
} // namespace net
} // namespace oct
