cmake_minimum_required(VERSION 3.14)

project(ssl_server)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/)

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SET(CMAKE_C_FLAGS "$-std=c11 -O2 -W -w")
if (CMAKE_BUILD_TYPE STREQUAL Release)
	set(CMAKE_CXX_FLAGS "-W -w -DNDEBUG -O2")
else()
	set(CMAKE_CXX_FLAGS "-W -w -D_DEBUG -g")
endif()
#SET(CMAKE_CXX_FLAGS "-W -w")
#set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -O0 -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

find_package(PkgConfig REQUIRED)
if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config not found")
endif()

pkg_search_module(LIBUV REQUIRED libuv)
if(NOT LIBUV_FOUND)
    message(FATAL_ERROR "libuv not found")
endif()
include_directories(${LIBUV_INCLUDE_DIRS})
link_directories(${LIBUV_LIBRARY_DIRS})

pkg_search_module(OPENSSL REQUIRED openssl)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "openssl not found")
endif()
include_directories(${OPENSSL_INCLUDE_DIRS})
link_directories(${OPENSSL_LIBRARY_DIRS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/objs)

include_directories(/usr/local/include)
include_directories(${CMAKE_SOURCE_DIR})

SET(EXTRA_LIBS uvnet)
SET(LIBS pthread uv ssl crypto)

file(GLOB SRCS "echo_server.cpp" "*.cpp")


message(STATUS ${SRCS})

link_directories(${CMAKE_SOURCE_DIR}/objs)
link_directories(/usr/local/lib)
link_directories(/usr/local/lib64)

# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} ${LIBS})
