#ifndef __IP_H
#define __IP_H

#include "netif.h"
#include "ethernet.h"
#include "list.h"

#define IP_ALEN 4 /* 单位是字节，即为4*8=32bit */
#define IP_VERSION_4 4


static inline int ipv4_is_loopback(unsigned int addr){
    return (addr & _htonl(0xff000000)) == _htonl(0x7f000000);
}

static inline int ipv4_is_nulticast(unsigned int addr){
    return (addr & _htonl(0xf0000000)) == _htonl(0xe0000000);
}

static inline int ipv4_is_broadcast(unsigned int addr){
    return addr == _htonl(0xffffffff);
}

/* 0.0.0.0表示本网络中的所有主机 */
static inline int ipv4_is_zeronet(unsigned int addr){
    return (addr & _htonl(0xff000000)) == _htonl(0x00000000);
}

#endif