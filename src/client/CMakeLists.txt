cmake_minimum_required(VERSION 3.13)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Asio REQUIRED)

project(octnet-tftp-libclient
    VERSION ${RELEASE_VERSION}
)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} INTERFACE)
add_library(octnet-tftp::libclient ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_NAME}
    INTERFACE
        defs.hpp
        deserializer.hpp
        file_io.hpp
        io.hpp
        make_unique.hpp
        netascii_io.hpp
        packet_builder.hpp
        packet_parser.hpp
        packet.hpp
        string_utils.hpp
)

target_link_libraries(${PROJECT_NAME} INTERFACE 3rdparty::asio)
target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)
