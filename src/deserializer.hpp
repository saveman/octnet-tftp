#pragma once

#include <exception>
#include <iostream>

#include <asio.hpp>

namespace oct
{
namespace net
{
namespace tftp
{

class deserialize_error : public std::runtime_error
{
public:
    deserialize_error(const char* message)
        : std::runtime_error(message)
    {
        // noop
    }
};

class deserializer
{
public:
    deserializer(const asio::const_buffer& buffer)
        : m_buffer(buffer)
    {
        auto data_ptr = reinterpret_cast<const char*>(buffer.data());
        m_current_ptr = data_ptr;
        m_end_ptr = data_ptr + buffer.size();
    }

    void ensure_all_read() const
    {
        if (has_more_bytes())
        {
            throw deserialize_error("too much data");
        }
    }

    bool has_more_bytes() const
    {
        return m_current_ptr < m_end_ptr;
    }

    void read_remaining_bytes(std::vector<std::uint8_t>& buffer)
    {
        buffer.insert(
            buffer.end(), reinterpret_cast<const uint8_t*>(m_current_ptr), reinterpret_cast<const uint8_t*>(m_end_ptr));
        m_current_ptr = m_end_ptr;
    }

    std::uint8_t read_uint8()
    {
        if (m_current_ptr >= m_end_ptr)
        {
            throw deserialize_error("not enough data");
        }
        return static_cast<std::uint8_t>(*m_current_ptr++);
    }

    std::uint16_t read_uint16()
    {
        std::uint16_t byte1 = read_uint8();
        std::uint16_t byte2 = read_uint8();
        return (byte1 << 8) | (byte2 << 0);
    }

    std::string read_string()
    {
        const char* start_ptr = m_current_ptr;
        while (read_uint8() != 0)
        {
            // continue
        }
        return std::string(start_ptr);
    }

private:
    const asio::const_buffer& m_buffer;
    const char* m_current_ptr;
    const char* m_end_ptr;
};

} // namespace tftp
} // namespace net
} // namespace oct
