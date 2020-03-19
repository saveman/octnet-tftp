#pragma once

#include <cstdint>

namespace oct
{
namespace net
{
namespace tftp
{

const std::uint16_t OP_RRQ = 1;
const std::uint16_t OP_WRQ = 2;
const std::uint16_t OP_DATA = 3;
const std::uint16_t OP_ACK = 4;
const std::uint16_t OP_ERROR = 5;

const std::uint16_t ERRCODE_UNDEFINED = 0;
const std::uint16_t ERRCODE_FILE_NOT_FOUND = 1;
const std::uint16_t ERRCODE_ACCESS_VIOLATION = 2;
const std::uint16_t ERRCODE_DISK_FULL = 3;
const std::uint16_t ERRCODE_ILLEGAL_OP = 4;
const std::uint16_t ERRCODE_UNKNOWN_TRANSFER_ID = 5;
const std::uint16_t ERRCODE_FILE_ALREADY_EXISTS = 6;
const std::uint16_t ERRCODE_FILE_NO_SUCH_USER = 7;

const std::uint16_t DEFAULT_TFTP_PORT = 69;

const std::size_t MAX_PACKET_SIZE = 4096;

const std::size_t DEFAULT_BLOCK_SIZE = 512;

const int DEFAULT_RETRY_TIMEOUT_SEC = 1;

const int DEFAULT_RETRY_COUNTER = 5;

} // namespace tftp
} // namespace net
} // namespace oct
