#include "server_app.hpp"

int main(int argc, char* argv[])
{
    oct::net::tftp::server_app server(argc, argv);
    return server.run();
}
