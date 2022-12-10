cmake_minimum_required(VERSION 3.16)
project(libnice C)

#option(ENABLE_MBEDTLS "enable mbedtls library" OFF)
option(ENABLE_EXAMPLE "enable example for ice" ON)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(LINK_LIB_LIST "")
find_package(Glib REQUIRED)
if(Glib_FOUND)
    message("-- found Glib, include dir: ${Glib_PROCESS_INCLUDES}, library: ${Glib_PROCESS_LIBS}")
    include_directories(${Glib_PROCESS_INCLUDES})
    list(APPEND LINK_LIB_LIST ${Glib_PROCESS_LIBS})
endif()


find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    list(APPEND LINK_LIB_LIST ${OPENSSL_LIBRARIES})
    add_definitions(-DHAVE_OPENSSL)
endif()


include(CheckIncludeFile)
check_include_file(netdb.h HAVE_NETDB_H)

if(HAVE_NETDB_H)
    add_definitions(-DHAVE_NETDB_H)
endif()



set(SUBDIR agent stun socket random)
foreach(item ${SUBDIR})
    add_subdirectory(${item})
endforeach()




if(ENABLE_EXAMPLE)
    add_subdirectory(examples)
endif()
