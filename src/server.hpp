#pragma once

#include <iostream>
#include <memory>
#include <set>

#include <asio.hpp>

#include "make_unique.hpp"
#include "read_connection.hpp"
#include "request_handler.hpp"
#include "server_acceptor.hpp"
#include "server_settings.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class server : private request_handler
{
public:
    server()
        : m_io_context()
    {
        m_settings = std::make_shared<server_settings>();
        m_settings->m_server_port = 6969;
        m_settings->m_root_path = "testdata";
    }

    int run()
    {
        try
        {
            start();

            m_io_context.run();

            return EXIT_SUCCESS;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

private:
    void start()
    {
        try
        {
            request_handler& handler = *this;
            m_acceptor = stdext::make_unique<server_acceptor>(m_io_context, handler);
            m_acceptor->start(m_settings->m_server_port);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Start failed: " << e.what() << std::endl;
        }
    }

    void handle_server_packet(std::shared_ptr<packet> packet, const asio::ip::udp::endpoint& send_endpoint) final
    {
        try
        {
            if (packet->m_op == OP_RRQ)
            {
                auto req_packet = std::static_pointer_cast<packet_file_req>(packet);

                request_handler& handler = *this;

                auto new_connection
                    = std::make_shared<read_connection>(handler, m_io_context, m_settings, req_packet, send_endpoint);

                connection_created(new_connection);
            }
            else
            {
                std::cerr << "Unsupported packet type: " << packet->m_op << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Packet processing failed: " << e.what() << std::endl;
        }
    }

    void connection_created(std::shared_ptr<read_connection> connection)
    {
        std::cout << "Connection created: " << connection.get() << std::endl;

        m_connections.insert(connection);

        connection->start();
    }

    void connection_terminated(std::shared_ptr<read_connection> connection) final
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

    std::shared_ptr<server_settings> m_settings;
    asio::io_context m_io_context;

    std::unique_ptr<server_acceptor> m_acceptor;
    std::set<std::shared_ptr<read_connection>> m_connections;
};

} // namespace tftp
} // namespace net
} // namespace oct
