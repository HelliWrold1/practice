cmake_minimum_required(VERSION 3.16)
project(practice C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(practice
        threadpool/threadpool.c
        threadpool/threadpool.h socket/server.c socket/client.c socket/socket.h main.c socket_eventio/server.c)
add_executable(practice_eventio
        socket_eventio/server.c log/log.c)

add_executable(practice_socketclient
        socket_eventio/client.c log/log.c)