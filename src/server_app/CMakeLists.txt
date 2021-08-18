cmake_minimum_required(VERSION 3.13)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Asio REQUIRED)

project(octnet-tftpd
    VERSION ${RELEASE_VERSION}
)

include(GNUInstallDirs)

add_executable(${PROJECT_NAME})
#add_executable(octnet-tftp::tftpd ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_NAME}
    PRIVATE
        file_data_io_manager.hpp
        main.cpp
        server_app.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 3rdparty::asio)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE octnet-tftp-libcommon)
target_link_libraries(${PROJECT_NAME} PRIVATE octnet-tftp-libserver)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT server
)
