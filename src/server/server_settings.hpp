#pragma once

#include <cstdint>
#include <string>

#include "defs.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

struct server_settings
{
    server_settings()
        : m_server_port(DEFAULT_TFTP_PORT)
        , m_root_path()
    {
        // noop
    }

    std::uint16_t m_server_port;
    std::string m_root_path;
};

} // namespace tftp
} // namespace net
} // namespace oct
