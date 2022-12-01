aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOCKET_SRC)
add_library(socket STATIC ${SOCKET_SRC})
target_include_directories(socket PUBLIC ${PROJECT_SOURCE_DIR})