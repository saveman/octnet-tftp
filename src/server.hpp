#pragma once

#include <iostream>
#include <memory>
#include <set>

#include <asio.hpp>

#include "default_io_manager.hpp"
#include "make_unique.hpp"
#include "read_connection.hpp"
#include "request_handler.hpp"
#include "server_acceptor.hpp"
#include "server_settings.hpp"
#include "write_connection.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class server : private request_handler
{
public:
    server(asio::io_context& io_context, server_settings& settings, io_manager& io_manager)
        : m_io_context(io_context)
        , m_settings(settings)
        , m_io_manager(io_manager)
        , m_acceptor(stdext::make_unique<server_acceptor>(m_io_context, get_handler()))
    {
        // noop
    }

    void start()
    {
        m_acceptor->start(m_settings.m_server_port);
    }

private:
    request_handler& get_handler()
    {
        return *this;
    }

    void handle_rrq_packet(std::shared_ptr<packet_file_req> packet, const asio::ip::udp::endpoint& client_endpoint)
    {
        auto new_connection
            = std::make_shared<read_connection>(get_handler(), m_io_manager, m_io_context, packet, client_endpoint);

        connection_created(new_connection);
    }

    void handle_wrq_packet(std::shared_ptr<packet_file_req> packet, const asio::ip::udp::endpoint& client_endpoint)
    {
        auto new_connection
            = std::make_shared<write_connection>(get_handler(), m_io_manager, m_io_context, packet, client_endpoint);

        connection_created(new_connection);
    }

    void handle_server_packet(std::shared_ptr<packet> packet, const asio::ip::udp::endpoint& client_endpoint) final
    {
        try
        {
            switch (packet->m_op)
            {
            case OP_RRQ:
                handle_rrq_packet(std::static_pointer_cast<packet_file_req>(packet), client_endpoint);
                break;

            case OP_WRQ:
                handle_wrq_packet(std::static_pointer_cast<packet_file_req>(packet), client_endpoint);
                break;

            default:
                std::cerr << "Unsupported packet type: " << packet->m_op << std::endl;
                break;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Packet processing failed: " << e.what() << std::endl;
        }
    }

    void connection_created(std::shared_ptr<connection> connection)
    {
        std::cout << "Connection created: " << connection.get() << std::endl;

        m_connections.insert(connection);

        connection->start();
    }

    void connection_terminated(std::shared_ptr<connection> connection) final
    {
        std::cout << "Connection terminated: " << connection.get() << std::endl;

        connection->stop();

        auto iter = m_connections.find(connection);
        if (iter != m_connections.end())
        {
            m_connections.erase(iter);
        }
        else
        {
            std::cerr << "Connection terminate request but connection not registered" << std::endl;
        }
    }

    asio::io_context& m_io_context;
    server_settings& m_settings;
    io_manager& m_io_manager;

    std::unique_ptr<server_acceptor> m_acceptor;
    std::set<std::shared_ptr<connection>> m_connections;
};

} // namespace tftp
} // namespace net
} // namespace oct
