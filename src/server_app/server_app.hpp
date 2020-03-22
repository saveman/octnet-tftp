#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include <asio.hpp>

#include "default_io_manager.hpp"
#include "server.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class server_app
{
public:
    server_app(int /*argc*/, char* /*argv*/[])
        : m_io_context()
        , m_signals(m_io_context, SIGTERM)
    {
        m_settings = create_settings();
        m_io_manager = create_io_manager(*m_settings);
        m_server = stdext::make_unique<server>(m_io_context, *m_settings, *m_io_manager);
    }

    int run()
    {
        try
        {
            m_signals.async_wait(std::bind(&server_app::on_signal, this, std::placeholders::_1, std::placeholders::_2));

            m_server->start();

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

    static std::unique_ptr<server_settings> create_settings()
    {
        auto settings = stdext::make_unique<server_settings>();
        settings->m_server_port = 6969;
        settings->m_root_path = "testdata";
        return settings;
    }

    static std::unique_ptr<io_manager> create_io_manager(server_settings& settings)
    {
        return stdext::make_unique<default_io_manager>(settings.m_root_path);
    }

    asio::io_context m_io_context;
    asio::signal_set m_signals;

    std::unique_ptr<server_settings> m_settings;
    std::unique_ptr<io_manager> m_io_manager;
    std::unique_ptr<server> m_server;
};

} // namespace tftp
} // namespace net
} // namespace oct
