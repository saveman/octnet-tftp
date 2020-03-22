#pragma once

#include <functional>
#include <iostream>
#include <string>

#include <asio.hpp>

#include "defs.hpp"
#include "packet_parser.hpp"
#include "request_handler.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class server_acceptor : public std::enable_shared_from_this<server_acceptor>
{
public:
    server_acceptor(asio::io_context& io_context, request_handler& handler)
        : m_io_context(io_context)
        , m_handler(handler)
        , m_server_socket(io_context)
    {
        // noop
    }

    void start(std::uint16_t server_port)
    {
        asio::ip::udp::endpoint server_endpoint(asio::ip::address_v4::any(), server_port);

        m_server_socket.open(server_endpoint.protocol());
        m_server_socket.set_option(asio::socket_base::reuse_address(true));
        m_server_socket.bind(server_endpoint);

        request_receive();
    }

    void stop()
    {
        asio::error_code ec;

        if (m_server_socket.is_open())
        {
            m_server_socket.close(ec);
            if (ec)
            {
                std::cerr << "Socket close failed: " << ec << std::endl;
            }
        }
    }

private:
    void request_receive()
    {
        m_server_socket.async_receive_from(asio::buffer(m_packet_buffer), m_packet_sender_endpoint,
            std::bind(&server_acceptor::on_packet_received, shared_from_this(), std::placeholders::_1,
                std::placeholders::_2));
    }

    void on_packet_received(const asio::error_code& ec, std::size_t bytes_received)
    {
        if (!ec && (bytes_received > 0))
        {
            asio::const_buffer buffer(m_packet_buffer.data(), bytes_received);

            process_initial_packet(buffer, m_packet_sender_endpoint);
        }
        else
        {
            std::cerr << "Error occurred: " << ec << std::endl;
        }

        request_receive();
    }

    void process_initial_packet(const asio::const_buffer& buffer, const asio::ip::udp::endpoint& sender_endpoint)
    {
        std::cout << "Packet received with " << buffer.size() << " bytes" << std::endl;
        std::cout << "Sender: " << m_packet_sender_endpoint << std::endl;

        auto packet = packet_parser::parse_packet(buffer);
        if (packet)
        {
            m_handler.handle_server_packet(packet, sender_endpoint);
        }
        else
        {
            std::cerr << "Cannot parse packet" << std::endl;
        }
    }

    asio::io_context& m_io_context;
    request_handler& m_handler;

    asio::ip::udp::socket m_server_socket;

    std::array<std::uint8_t, MAX_PACKET_SIZE> m_packet_buffer;
    asio::ip::udp::endpoint m_packet_sender_endpoint;
};

} // namespace tftp
} // namespace net
} // namespace oct
