cmake_minimum_required(VERSION 3.12)
project(Tinytcpipstack C)

set(CMAKE_C_STANDARD 99)

add_library(Tinytcpipstack library.c library.h test.c)
add_executable(MAIN1 utils/checksum.c utils/ring_buffer.c utils/ring_buffer.h utils/lib.c utils/lib.h)