#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include <asio.hpp>

#include "connection.hpp"
#include "defs.hpp"
#include "file_io.hpp"
#include "make_unique.hpp"
#include "packet.hpp"
#include "packet_builder.hpp"
#include "packet_parser.hpp"
#include "request_handler.hpp"
#include "server_settings.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class read_connection : public connection
{
public:
    read_connection(request_handler& handler, io_manager& io_manager, asio::io_context& io_context,
        std::shared_ptr<const packet_file_req> request_packet, const asio::ip::udp::endpoint& requesting_endpoint)
        : m_handler(handler)
        , m_io_manager(io_manager)
        , m_connection_socket(io_context)
        , m_send_timeout_timer(io_context)
        , m_request_packet(request_packet)
        , m_client_endpoint(requesting_endpoint)
        , m_reader()
        , m_last_packet_sent(false)
        , m_last_sent_packet_id(0)
        , m_response_expected(false)
        , m_retry_counter(0)
    {
        // noop
    }

    void start() final
    {
        asio::ip::udp::endpoint connection_endpoint(asio::ip::address_v4::any(), 0);

        m_connection_socket.open(connection_endpoint.protocol());
        m_connection_socket.set_option(asio::socket_base::reuse_address(true));
        m_connection_socket.bind(connection_endpoint);

        m_reader = m_io_manager.create_reader(m_request_packet->m_filename, m_request_packet->m_mode);

        send_first_packet();
    }

    void stop() final
    {
        asio::error_code ec;

        if (m_connection_socket.is_open())
        {
            m_connection_socket.close(ec);
            if (ec)
            {
                std::cerr << "Socket close failed: " << ec << std::endl;
            }
        }

        m_send_timeout_timer.cancel();
    }

private:
    void send_first_packet()
    {
        if (!m_reader)
        {
            packet_error packet;
            packet.m_op = OP_ERROR;
            packet.m_error_code = ERRCODE_ACCESS_VIOLATION;
            packet.m_error_message = "invalid path or mode";

            send_packet(packet, false, 0);
        }
        else if (!m_reader->is_open())
        {
            packet_error packet;
            packet.m_op = OP_ERROR;
            packet.m_error_code = ERRCODE_FILE_NOT_FOUND;
            packet.m_error_message = "file not found";

            send_packet(packet, false, 0);
        }
        else
        {
            send_next_data_packet();
        }
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
        m_connection_socket.async_send_to(asio::const_buffer(m_out_packet_data.data(), m_out_packet_data.size()),
            m_client_endpoint,
            std::bind(&read_connection::on_packet_sent, shared_from_base<read_connection>(), std::placeholders::_1,
                std::placeholders::_2));
    }

    void send_next_data_packet()
    {
        std::size_t bytes_read = 0;

        packet_data packet;
        packet.m_op = OP_DATA;
        packet.m_block_no = ++m_last_sent_packet_id;
        packet.m_data.resize(DEFAULT_DATA_SIZE);

        if (!m_reader->read(packet.m_data.data(), packet.m_data.size(), bytes_read))
        {
            std::cerr << "Read failed" << std::endl;

            packet_error packet;
            packet.m_op = OP_ERROR;
            packet.m_error_code = ERRCODE_FILE_NOT_FOUND;
            packet.m_error_message = "invalid path";

            send_packet(packet, false, 0);

            return;
        }

        std::cout << "Bytes read: " << bytes_read << std::endl;

        packet.m_data.resize(bytes_read, 0);

        if (bytes_read < DEFAULT_DATA_SIZE)
        {
            m_last_packet_sent = true;
        }

        send_packet(packet, true, DEFAULT_RETRY_COUNTER);
    }

    void request_next_receive()
    {
        m_connection_socket.async_receive_from(asio::buffer(m_in_packet_data), m_received_packet_endpoint,
            std::bind(&read_connection::on_packet_received, shared_from_base<read_connection>(), std::placeholders::_1,
                std::placeholders::_2));
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
            terminate();
            return;
        }

        std::cout << "Packet sent: " << bytes_transferred << std::endl;

        if (m_response_expected)
        {
            m_send_timeout_timer.expires_after(std::chrono::seconds(DEFAULT_RETRY_TIMEOUT_SEC));
            m_send_timeout_timer.async_wait(std::bind(
                &read_connection::on_send_timeout, shared_from_base<read_connection>(), std::placeholders::_1));
            request_next_receive();
        }
        else
        {
            terminate();
        }
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
            terminate();
        }
    }

    void process_ack_received(std::shared_ptr<packet_ack> packet)
    {
        std::cout << "ACK received: " << packet->m_block_no << std::endl;

        if (packet->m_block_no != m_last_sent_packet_id)
        {
            std::cerr << "ACK with bad block no received: " << packet->m_block_no << std::endl;
            return;
        }

        m_send_timeout_timer.cancel();

        if (m_last_packet_sent)
        {
            terminate();
            return;
        }

        send_next_data_packet();
    }

    void process_error_received(std::shared_ptr<packet_error> packet)
    {
        std::cout << "ERROR received: " << packet->m_error_code << ' ' << packet->m_error_message << std::endl;
        terminate();
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
            terminate();
            return;
        }

        std::cout << "Packet received: " << bytes_received << std::endl;

        if (m_client_endpoint != m_received_packet_endpoint)
        {
            std::cerr << "Received packet from unexpected source: " << ec << std::endl;
            return;
        }

        auto packet = packet_parser::parse_packet(asio::const_buffer(m_in_packet_data.data(), bytes_received));
        if (!packet)
        {
            std::cerr << "Invalid packet received of size: " << bytes_received << std::endl;
            return;
        }

        switch (packet->m_op)
        {
        case OP_ACK:
            process_ack_received(std::static_pointer_cast<packet_ack>(packet));
            return;

        case OP_ERROR:
            process_error_received(std::static_pointer_cast<packet_error>(packet));
            return;

        default:
            std::cerr << "Unexpected packet type: " << packet->m_op << std::endl;
            return;
        }
    }

    void terminate()
    {
        m_handler.connection_terminated(shared_from_base<read_connection>());
    }

    request_handler& m_handler;
    io_manager& m_io_manager;

    asio::ip::udp::socket m_connection_socket;
    asio::system_timer m_send_timeout_timer;

    std::shared_ptr<const packet_file_req> m_request_packet;

    const asio::ip::udp::endpoint m_client_endpoint;

    std::unique_ptr<reader> m_reader;
    bool m_last_packet_sent;

    std::uint16_t m_last_sent_packet_id;

    asio::ip::udp::endpoint m_received_packet_endpoint;

    bool m_response_expected;
    int m_retry_counter;

    std::vector<std::uint8_t> m_out_packet_data;
    std::array<std::uint8_t, MAX_RECV_PACKET_SIZE> m_in_packet_data;
};

} // namespace tftp
} // namespace net
} // namespace oct
