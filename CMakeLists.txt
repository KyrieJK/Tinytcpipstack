cmake_minimum_required(VERSION 3.12)
project(Tinytcpipstack C)

set(CMAKE_C_STANDARD 99)

add_library(Tinytcpipstack library.c library.h)
add_executable(MAIN1 test.c include/netif.h include/list.h include/route.h include/socket.h include/sock.h include/inet.h include/ethernet.h core/tap.c include/tap.h core/veth.c include/netconfig.h core/pkb.c core/dllnet.c core/socket/inet.c core/socket/sock.c core/socket/socket.c core/ip/route.c)
add_executable(MAIN2 utils/checksum.c utils/ring_buffer.c utils/ring_buffer.h utils/lib.c include/lib.h )