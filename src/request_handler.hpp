#pragma once

#include <memory>
#include <string>

#include <asio.hpp>

namespace oct
{
namespace net
{
namespace tftp
{

class packet;
class read_connection;

class request_handler
{
public:
    virtual ~request_handler() = default;

    virtual void handle_server_packet(std::shared_ptr<packet> packet, const asio::ip::udp::endpoint& send_endpoint) = 0;

    virtual void connection_terminated(std::shared_ptr<read_connection> connection) = 0;
};

} // namespace tftp
} // namespace net
} // namespace oct
