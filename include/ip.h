#ifndef __IP_H
#define __IP_H

#include "netif.h"
#include "ethernet.h"
#include "list.h"

#define IP_ALEN 4 /* 单位是字节，即为4*8=32bit */
#define IP_VERSION_4 4


#define ipv4_is_loopback(addr) ((addr & _htonl(0xff000000)) == _htonl(0x7f000000))
#define ipv4_is_multicast(addr) ((addr & _htonl(0xf0000000)) == _htonl(0xe0000000))
#define ipv4_is_broadcast(addr) (addr == _htonl(0xffffffff))
#define ipv4_is_zeronet(addr) ((addr & _htonl(0xff000000)) == _htonl(0x00000000)) /* 0.0.0.0表示本网络中的所有主机 */

#endif