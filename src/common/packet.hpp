#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace oct
{
namespace net
{
namespace tftp
{

struct packet
{
    std::uint16_t m_op;
};

struct packet_file_req : public packet
{
    std::string m_filename;
    std::string m_mode;
    std::map<std::string, std::string> m_options;
};

struct packet_data : public packet
{
    std::uint16_t m_block_no;
    std::vector<std::uint8_t> m_data;
};

struct packet_ack : public packet
{
    std::uint16_t m_block_no;
};

struct packet_error : public packet
{
    std::uint16_t m_error_code;
    std::string m_error_message;
};

} // namespace tftp
} // namespace net
} // namespace oct
