#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include <asio.hpp>

#include "client_get.hpp"
#include "client_put.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class client_app
{
public:
    client_app(int /*argc*/, char* /*argv*/[])
        : m_io_context()
        , m_signals(m_io_context, SIGTERM)
    {
        // m_server = stdext::make_unique<server>(m_io_context, *m_settings, *m_io_manager);
    }

    int run()
    {
        request this_request;

        this_request.m_type = request_type::GET;
        // this_request.m_type = request_type::PUT;
        this_request.m_host = "localhost";
        this_request.m_port = 9999;
        this_request.m_remote_filename = "file1.txt";
        this_request.m_local_path = "testdata/client2.txt";
        this_request.m_mode = "octet";

        try
        {
            m_signals.async_wait(std::bind(&client_app::on_signal, this, std::placeholders::_1, std::placeholders::_2));

            if (this_request.m_type == request_type::GET)
            {
                auto test_client1 = std::make_shared<client_get>(m_io_context, this_request);
                test_client1->start();
            }
            else
            {
                auto test_client2 = std::make_shared<client_put>(m_io_context, this_request);
                test_client2->start();
            }

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
    void on_signal(const asio::error_code& ec, int signal_number)
    {
        if (ec)
        {
            std::cerr << "Signal error: " << ec << std::endl;
            return;
        }

        if (signal_number == SIGTERM)
        {
            std::cout << "Terminate requested" << std::endl;
            m_io_context.stop();
        }
        else
        {
            std::cerr << "Unexpected signal: " << signal_number << std::endl;
        }
    }

    asio::io_context m_io_context;
    asio::signal_set m_signals;
    // std::unique_ptr<server> m_server;
};

} // namespace tftp
} // namespace net
} // namespace oct
