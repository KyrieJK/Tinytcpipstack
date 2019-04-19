cmake_minimum_required(VERSION 3.12)
project(Tinytcpipstack C)

set(CMAKE_C_STANDARD 99)

add_library(Tinytcpipstack library.c library.h test.c)