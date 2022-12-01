aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} STUN_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/usages STUN_SRC)
add_library(stun STATIC ${STUN_SRC})
target_include_directories(agent PUBLIC ${PROJECT_SOURCE_DIR})