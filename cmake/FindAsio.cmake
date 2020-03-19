include(FindPackageHandleStandardArgs)

if(DEFINED ASIO_ROOT_DIR)
    set(ASIO_ROOT_INCLUDE_DIR "${ASIO_ROOT_DIR}/asio/include") 
endif()

find_path(ASIO_INCLUDE_DIR
    NAMES
        asio.hpp
    HINTS
        ${ASIO_ROOT_INCLUDE_DIR}
)

set(ASIO_INCLUDE_DIRS ${ASIO_INCLUDE_DIR})

find_package_handle_standard_args(Asio DEFAULT_MSG
    ASIO_INCLUDE_DIR
)

mark_as_advanced(ASIO_INCLUDE_DIR)

if(Asio_FOUND AND NOT TARGET 3rdparty::asio)
    add_library(3rdparty::asio INTERFACE IMPORTED)
    target_include_directories(3rdparty::asio INTERFACE "${ASIO_INCLUDE_DIR}")
    target_compile_definitions(3rdparty::asio INTERFACE ASIO_STANDALONE)
    target_compile_definitions(3rdparty::asio INTERFACE ASIO_NO_DEPRECATED)
endif()
