cmake_minimum_required(VERSION 3.16)
project(practice C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(practice
        threadpool/threadpool.c
        threadpool/threadpool.h socket/server.c socket/client.c socket/socket.h main.c)
