//
// Created by 张家望 on 2020/1/6.
//
#ifndef TINYTCPIPSTACK_UDP_H
#define TINYTCPIPSTACK_UDP_H

#include "sock.h"

/**
 * UDP结构体
 * UDP长度字段指的是UDP首部和UDP数据的总字节数，该字段的最小值为8，即无UDP数据，仅有UDP头部
 * IP数据报长度指的是整个IP数据报的总长度，因此UDP长度是IP长度减去IP首部长度
 */
struct udp{
    unsigned short src; //16位源地址
    unsigned short dst; //16位目的地址
    unsigned short length; //16位UDP长度
    unsigned short checksum; //16位UDP校验和
    unsigned char data[0]; //UDP数据域
} __attribute__((packed));

/**
 * udp_sock结构由sock结构扩展而来
 */
struct udp_sock{
    struct sock sk;
};

#define UDP_HRD_SZ  sizeof(struct udp)
#define ip2udp(ip)  ((struct udp *)ipdata(ip))
#define UDP_MAX_BUFSZ   (0xffff-UDP_HRD_SZ)
#define UDP_DEFAULT_TTL 64

#endif //TINYTCPIPSTACK_UDP_H
