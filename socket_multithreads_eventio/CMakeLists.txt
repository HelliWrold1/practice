cmake_minimum_required(VERSION 3.16)
project(practice C)
set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(multithreads_eventio_client client.c)
target_link_libraries(multithreads_eventio_client log)

add_executable(multithreads_eventio_server server.c)
target_link_libraries(multithreads_eventio_server pthread log)
