cmake_minimum_required(VERSION 3.13)

project(octargs
    VERSION ${RELEASE_VERSION}
)

include(GNUInstallDirs)

add_executable(${PROJECT_NAME})
add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# target_include_directories(${PROJECT_NAME}
#     INTERFACE
#         ${CMAKE_CURRENT_SOURCE_DIR}
# )

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

# install(FILES ${${PROJECT_NAME}_HEADERS}
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/octargs
#     COMPONENT dev
# )
# 
# install(FILES ${${PROJECT_NAME}_HEADERS_INTERNAL}
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/octargs/internal
#     COMPONENT dev
# )
