cmake_minimum_required(VERSION 3.13)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(ASIO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/asio")
find_package(Asio REQUIRED)

project(octnet-tftpd
    VERSION ${RELEASE_VERSION}
)

include(GNUInstallDirs)

add_executable(${PROJECT_NAME})
add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# target_include_directories(${PROJECT_NAME}
#     PRIVATE
# )

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 3rdparty::asio
)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

# install(FILES ${${PROJECT_NAME}_HEADERS}
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/octargs
#     COMPONENT dev
# )
# 
# install(FILES ${${PROJECT_NAME}_HEADERS_INTERNAL}
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/octargs/internal
#     COMPONENT dev
# )
