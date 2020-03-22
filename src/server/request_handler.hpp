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
class connection;

class request_handler
{
public:
    virtual ~request_handler() = default;

    virtual void handle_server_packet(std::shared_ptr<packet> packet, const asio::ip::udp::endpoint& client_endpoint)
        = 0;

    virtual void connection_terminated(std::shared_ptr<connection> connection) = 0;
};

} // namespace tftp
} // namespace net
} // namespace oct
