cmake_minimum_required(VERSION 3.10)

project(test)

add_subdirectory(echo_server)
add_subdirectory(send_udp_mutlithread)
add_subdirectory(tcp_client)
add_subdirectory(udp_server)
add_subdirectory(ssl_server)
