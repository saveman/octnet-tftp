#pragma once

#include <cctype>
#include <string>

namespace oct
{
namespace net
{
namespace tftp
{

bool equal_ignore_case(const std::string& a, const std::string& b)
{
    const std::size_t a_size = a.size();

    if (b.size() != a_size)
    {
        return false;
    }

    for (std::size_t i = 0; i < a_size; ++i)
    {
        if (std::tolower(static_cast<uint8_t>(a[i])) != std::tolower(static_cast<uint8_t>(b[i])))
        {
            return false;
        }
    }

    return true;
}

} // namespace tftp
} // namespace net
} // namespace oct
