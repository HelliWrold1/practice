cmake_minimum_required(VERSION 3.16)
project(practice C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_subdirectory(log)
add_subdirectory(threadpool)
add_subdirectory(socket_multithreads)
add_subdirectory(socket_eventio)
add_subdirectory(socket_multithreads_eventio)

add_executable(practice_multithreads_select
        socket_multithreads_eventio/server.c)

target_link_libraries(practice_multithreads_select log)