cmake_minimum_required(VERSION 3.30)
project(Everything_in_C C)

set(CMAKE_C_STANDARD 17)

add_executable(Everything_in_C main.c
        memory.h)
