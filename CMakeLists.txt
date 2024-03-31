cmake_minimum_required(VERSION 3.16)
project(practice C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(practice_multithreads_server
        threadpool/threadpool.c
        socket_multithreads/server.c socket_multithreads/socket.h main.c)

add_executable(practice_multithreads_client
        socket_multithreads/client.c socket_multithreads/socket.h socket_multithreads/server.c threadpool/threadpool.c)

add_executable(practice_eventio
        socket_eventio/server.c log/log.c)

add_executable(practice_socketclient
        socket_eventio/client.c log/log.c)