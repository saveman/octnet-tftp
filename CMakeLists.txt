cmake_minimum_required(VERSION 3.13)

option(BUILD_EXAMPLES  "Build the examples"       ON)
option(ENABLE_COVERAGE "Enable coverage analysis" ON)

if("${RELEASE_VERSION}" STREQUAL "")
    set(RELEASE_VERSION "0.0.0")
endif()

project(octargs_main
    VERSION ${RELEASE_VERSION}
    DESCRIPTION "C++ command line argument parsing library"
    HOMEPAGE_URL "https://github.com/saveman/octargs"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src)
