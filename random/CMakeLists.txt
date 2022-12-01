aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RANDOM_SRC)
add_library(random STATIC ${RANDOM_SRC})
target_include_directories(random PUBLIC ${PROJECT_SOURCE_DIR})