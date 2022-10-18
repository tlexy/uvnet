cmake_minimum_required(VERSION 3.10)

project(uvnet)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -O0")

FILE (GLOB HEADERS "./core/*.h" "./core/httpparser/*.h" "./core/*.hpp" "./server/*.h" "./utils/*.h" "utils/*.hpp" "./websocket/*.h", "./tls/*.h")

FILE (GLOB SRCS "./core/*.cpp" "./server/*.cpp" "./utils/*.cpp" "./websocket/*.cpp", "./tls/*.cpp")

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../../libs")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/objs)
include_directories(${CMAKE_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SRCS})

add_subdirectory(test)
