#include "client_app.hpp"

int main(int argc, char* argv[])
{
    oct::net::tftp::client_app server(argc, argv);
    return server.run();
}
