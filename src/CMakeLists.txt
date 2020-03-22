cmake_minimum_required(VERSION 3.13)

set(ASIO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/asio")

project(octnet-tftpd
    VERSION ${RELEASE_VERSION}
)

add_subdirectory(common)
#add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(server_app)
add_subdirectory(client_app)
