cmake_minimum_required(VERSION 3.16)
project(practice C)
set(CMAKE_C_STANDARD 11)

include_directories(.)

add_library(log SHARED log.c)

