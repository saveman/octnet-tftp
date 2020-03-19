#pragma once

#include <asio.hpp>
#include <iostream>

#include "defs.hpp"
#include "deserializer.hpp"
#include "packet.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class packet_parser
{
public:
    static std::shared_ptr<packet> parse_packet(const asio::const_buffer& buffer)
    {
        try
        {
            deserializer deserializer(buffer);

            auto op = deserializer.read_uint16();
            switch (op)
            {
            case OP_RRQ:
            case OP_WRQ:
                return parse_file_req(op, deserializer);
            case OP_DATA:
                return parse_data(deserializer);
            case OP_ACK:
                return parse_ack(deserializer);
            case OP_ERROR:
                return parse_error(deserializer);
            default:
                // TODO: log message
                std::cerr << "Cannot parse - unknown op: " << op << std::endl;
                return nullptr;
            }
        }
        catch (const deserialize_error& exc)
        {
            // TODO: log message
            std::cerr << "Packet deserialization failed: " << exc.what() << std::endl;
            return nullptr;
        }
    }

private:
    static std::shared_ptr<packet> parse_file_req(std::uint16_t op, deserializer& deserializer)
    {
        auto packet = std::make_shared<packet_file_req>();

        packet->m_op = op;
        packet->m_filename = deserializer.read_string();
        packet->m_mode = deserializer.read_string();

        while (deserializer.has_more_bytes())
        {
            auto name = deserializer.read_string();
            auto value = deserializer.read_string();

            packet->m_options.emplace(name, value);
        }
        deserializer.ensure_all_read();

        return packet;
    }

    static std::shared_ptr<packet> parse_wrq(deserializer& deserializer)
    {
        auto packet = std::make_shared<packet_file_req>();

        packet->m_op = OP_WRQ;
        packet->m_filename = deserializer.read_string();
        packet->m_mode = deserializer.read_string();

        while (deserializer.has_more_bytes())
        {
            auto name = deserializer.read_string();
            auto value = deserializer.read_string();

            packet->m_options.emplace(name, value);
        }
        deserializer.ensure_all_read();

        return packet;
    }

    static std::shared_ptr<packet> parse_data(deserializer& deserializer)
    {
        auto packet = std::make_shared<packet_data>();

        packet->m_op = OP_DATA;
        packet->m_block_no = deserializer.read_uint16();
        deserializer.read_remaining_bytes(packet->m_data);
        deserializer.ensure_all_read();

        return packet;
    }

    static std::shared_ptr<packet> parse_ack(deserializer& deserializer)
    {
        auto packet = std::make_shared<packet_ack>();

        packet->m_op = OP_ACK;
        packet->m_block_no = deserializer.read_uint16();
        deserializer.ensure_all_read();

        return packet;
    }

    static std::shared_ptr<packet> parse_error(deserializer& deserializer)
    {
        auto packet = std::make_shared<packet_error>();

        packet->m_op = OP_ERROR;
        packet->m_error_code = deserializer.read_uint16();
        packet->m_error_message = deserializer.read_string();
        deserializer.ensure_all_read();

        return packet;
    }
};

} // namespace tftp
} // namespace net
} // namespace oct
