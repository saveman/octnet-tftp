#pragma once

#include <cstdint>
#include <string>

namespace oct
{
namespace net
{
namespace tftp
{

enum class request_type
{
    GET,
    PUT,
};

struct request
{
    request()
        : m_type(request_type::GET)
        , m_port(0)
    {
        // noop
    }

    request_type m_type;
    std::string m_host;
    std::uint16_t m_port;
    std::string m_remote_filename;
    std::string m_local_path;
    std::string m_mode;
};

} // namespace tftp
} // namespace net
} // namespace oct
