cmake_minimum_required(VERSION 3.16)
project(practice C)
set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(multithreads_server server.c runServer.c)

add_executable(multithreads_client server.c client.c)

target_link_libraries(multithreads_server threadpool)

target_link_libraries(multithreads_client threadpool)

