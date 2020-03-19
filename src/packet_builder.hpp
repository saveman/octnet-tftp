#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <asio.hpp>

#include "defs.hpp"
#include "packet.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class serializer
{
public:
    serializer(std::vector<std::uint8_t>& buffer)
        : m_buffer(buffer)
    {
        // noop
    }

    void write_uint8(std::uint8_t value)
    {
        m_buffer.emplace_back(value);
    }

    void write_uint16(std::uint16_t value)
    {
        write_uint8((value >> 8) & 0xFF);
        write_uint8((value >> 0) & 0xFF);
    }

    void write_string(const std::string& str)
    {
        for (auto c : str)
        {
            m_buffer.emplace_back(static_cast<std::uint8_t>(c));
        }
        write_uint8(0);
    }

    void write_bytes(const std::vector<std::uint8_t>& bytes)
    {
        m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
    }

private:
    std::vector<std::uint8_t>& m_buffer;
};

class packet_builder
{
public:
    static std::vector<std::uint8_t> build_packet(const packet_data& packet)
    {
        std::vector<std::uint8_t> buffer;

        serializer serializer(buffer);

        serializer.write_uint16(packet.m_op);
        serializer.write_uint16(packet.m_block_no);
        serializer.write_bytes(packet.m_data);

        return buffer;
    }

    static std::vector<std::uint8_t> build_packet(const packet_error& packet)
    {
        std::vector<std::uint8_t> buffer;

        serializer serializer(buffer);

        serializer.write_uint16(packet.m_op);
        serializer.write_uint16(packet.m_error_code);
        serializer.write_string(packet.m_error_message);

        return buffer;
    }

    static std::vector<std::uint8_t> build_packet(const packet_ack& packet)
    {
        std::vector<std::uint8_t> buffer;

        serializer serializer(buffer);

        serializer.write_uint16(packet.m_op);
        serializer.write_uint16(packet.m_block_no);

        return buffer;
    }
};

} // namespace tftp
} // namespace net
} // namespace oct
