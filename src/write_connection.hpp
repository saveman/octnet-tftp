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

class write_connection : public connection
{
public:
    write_connection(request_handler& handler, io_manager& io_manager, asio::io_context& io_context,
        std::shared_ptr<const packet_file_req> request_packet, const asio::ip::udp::endpoint& requesting_endpoint)
        : m_handler(handler)
        , m_io_manager(io_manager)
        , m_connection_socket(io_context)
        , m_send_timeout_timer(io_context)
        , m_request_packet(request_packet)
        , m_client_endpoint(requesting_endpoint)
        , m_writer()
        , m_next_expected_packet_id(0)
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

        m_writer = m_io_manager.create_writer(m_request_packet->m_filename, m_request_packet->m_mode);

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
        if (!m_writer)
        {
            packet_error packet;
            packet.m_op = OP_ERROR;
            packet.m_error_code = ERRCODE_ACCESS_VIOLATION;
            packet.m_error_message = "invalid path";

            send_packet(packet, false, 0);
        }
        else if (!m_writer->is_open())
        {
            packet_error packet;
            packet.m_op = OP_ERROR;
            packet.m_error_code = ERRCODE_FILE_NOT_FOUND;
            packet.m_error_message = "invalid path";

            send_packet(packet, false, 0);
        }
        else
        {
            send_ack(0, false);
        }
    }

    void send_ack(std::uint32_t block_no, bool is_last)
    {
        std::cout << "Sending ACK: " << block_no << ' ' << is_last << std::endl;

        packet_ack packet;
        packet.m_op = OP_ACK;
        packet.m_block_no = block_no;

        m_next_expected_packet_id = block_no + 1;

        send_packet(packet, !is_last, DEFAULT_RETRY_COUNTER);
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
            std::bind(&write_connection::on_packet_sent, shared_from_base<write_connection>(), std::placeholders::_1,
                std::placeholders::_2));
    }

    void request_next_receive()
    {
        m_connection_socket.async_receive_from(asio::buffer(m_in_packet_data), m_received_packet_endpoint,
            std::bind(&write_connection::on_packet_received, shared_from_base<write_connection>(),
                std::placeholders::_1, std::placeholders::_2));
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
                &write_connection::on_send_timeout, shared_from_base<write_connection>(), std::placeholders::_1));
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

    void process_data_received(std::shared_ptr<packet_data> packet)
    {
        std::cout << "Data received: " << packet->m_block_no << std::endl;

        if (packet->m_block_no != m_next_expected_packet_id)
        {
            std::cerr << "Data with bad block no received: " << packet->m_block_no
                      << "; expected: " << m_next_expected_packet_id << std::endl;
            return;
        }

        m_send_timeout_timer.cancel();

        if (packet->m_data.size() > 0)
        {
            if (!m_writer->write(packet->m_data.data(), packet->m_data.size()))
            {
                std::cerr << "Write failed" << std::endl;

                packet_error packet;
                packet.m_op = OP_ERROR;
                packet.m_error_code = ERRCODE_FILE_NOT_FOUND;
                packet.m_error_message = "invalid path";

                send_packet(packet, false, 0);

                return;
            }
        }

        if (packet->m_data.size() == DEFAULT_DATA_SIZE)
        {
            send_ack(packet->m_block_no, false);
        }
        else
        {
            send_ack(packet->m_block_no, true);
            // TODO: dallying:
            //   The host acknowledging the final DATA packet may terminate its side
            //   of the connection on sending the final ACK.  On the other hand,
            //   dallying is encouraged.  This means that the host sending the final
            //   ACK will wait for a while before terminating in order to retransmit
            //   the final ACK if it has been lost.
        }
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

    void terminate()
    {
        m_handler.connection_terminated(shared_from_base<write_connection>());
    }

    request_handler& m_handler;
    io_manager& m_io_manager;

    asio::ip::udp::socket m_connection_socket;
    asio::system_timer m_send_timeout_timer;

    std::shared_ptr<const server_settings> m_settings;
    std::shared_ptr<const packet_file_req> m_request_packet;

    const asio::ip::udp::endpoint m_client_endpoint;

    std::unique_ptr<writer> m_writer;

    std::uint16_t m_next_expected_packet_id;

    asio::ip::udp::endpoint m_received_packet_endpoint;

    bool m_response_expected;
    int m_retry_counter;

    std::vector<std::uint8_t> m_out_packet_data;
    std::array<std::uint8_t, MAX_RECV_PACKET_SIZE> m_in_packet_data;
};

} // namespace tftp
} // namespace net
} // namespace oct
