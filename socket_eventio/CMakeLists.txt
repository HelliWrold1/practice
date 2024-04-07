cmake_minimum_required(VERSION 3.16)
project(practice C)
set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(eventio_server server.c)
target_link_libraries(eventio_server log)

add_executable(eventio_client client.c)
target_link_libraries(eventio_client log)
