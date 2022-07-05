cmake_minimum_required(VERSION 3.14)

project(uvnet)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -O0")

FILE (GLOB HEADERS "./core/*.h" "./server/*.h" "./utils/*.h" "utils/*.hpp")

FILE (GLOB SRCS "./core/*.cpp" "./server/*.cpp" "./utils/*.cpp")


#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../../libs")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/objs)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SRCS})

add_subdirectory(test)