aux_source_directory(${CMAKE_SOURCE_DIR}/examples EXAMPLE_SRC)
message("${LINK_LIB_LIST}")
foreach(item ${EXAMPLE_SRC})
    string(REGEX MATCH ".*[/\\](.*).c$" item_src ${item})
    message("  + ${CMAKE_MATCH_1}, ${item_src}")
    add_executable(${CMAKE_MATCH_1} ${item})
    target_link_libraries(${CMAKE_MATCH_1} agent stun socket random ${LINK_LIB_LIST})
    target_include_directories(${CMAKE_MATCH_1} INTERFACE ${CMAKE_SOURCE_DIR})
endforeach()