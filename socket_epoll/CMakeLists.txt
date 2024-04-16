cmake_minimum_required(VERSION 3.16)
project(practice C)
set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(epoll_server server.c)
target_link_libraries(epoll_server log)

add_executable(epoll_client client.c)
target_link_libraries(epoll_client log)
