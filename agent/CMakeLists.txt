aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} AGENT_SRC)
add_library(agent ${AGENT_SRC})
add_dependencies(agent socket stun)
target_include_directories(agent PUBLIC ${PROJECT_SOURCE_DIR})