#include "server.hpp"

int main(int /*argc*/, char* /*argv*/[])
{
    oct::net::tftp::server server;
    return server.run();
}
