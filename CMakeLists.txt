cmake_minimum_required(VERSION 3.26)
project(Salon C)

set(CMAKE_C_STANDARD 23)

add_executable(salon salon.c
        read-write.h
        read-write.c)
