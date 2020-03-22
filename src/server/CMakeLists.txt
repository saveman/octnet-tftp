cmake_minimum_required(VERSION 3.13)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Asio REQUIRED)

project(octnet-tftp-libserver
    VERSION ${RELEASE_VERSION}
)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} INTERFACE)
add_library(octnet-tftp::libserver ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_NAME}
    INTERFACE
        connection.hpp
        io_manager.hpp
        read_connection.hpp
        request_handler.hpp
        server_acceptor.hpp
        server_settings.hpp
        server.hpp
        write_connection.hpp
)

target_link_libraries(${PROJECT_NAME} INTERFACE 3rdparty::asio)
target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)
