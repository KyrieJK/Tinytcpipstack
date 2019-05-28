#ifndef __ARP_H
#define __ARP_H

#include "ethernet.h"
#include "list.h"

/* 定义L2、L3 协议类型 */
#define ARP_ETHERNET
#define ARP_IP

/* ARP高速缓存表项状态 */
#define ARP_FREE 1
#define ARP_PENDING 2
#define ARP_RESOLVED 3

#define ARP_REQ_RETRY 4

/* 硬件地址类型 */
#define ARPHRD_ETHER 1
/* ARP协议操作码(参考linux kernel中的定义) */
#define ARPOP_REQUEST         1     /* ARP request */
#define ARPOP_REPLY           2     /* ARP reply */
#define ARPOP_RREQUEST        3     /* RARP request */ 
#define ARPOP_RREPLY          4     /* RARP reply */
#define ARPOP_INREQUEST       8     /* InARP request */
#define ARPOP_INREPLY         9     /* InARP reply */

struct arp{
    unsigned short arp_hrd; /* hardware address type(L2) */
    unsigned short arp_pro; /* protocol address type(L3) */
    unsigned char arp_hlen; /* hardware address length */
    unsigned char arp_plen; /* protocol address length */
    unsigned short arp_op; /* ARP opcode */
#if defined(ARP_ETHERNET) && defined(ARP_IP) /* 本项目仅支持 以太网&IPv4 */
    unsigned char arp_sha[ETHERNET_ADDR_LEN]; /* sender hardware address */
    unsigned int arp_sip;                     /* sender ip addr */
    unsigned char arp_tha[ETHERNET_ADDR_LEN]; /* target hardware address */
    unsigned int arp_tip;                     /* target ip address */
#else
    unsigned char arp_data[0];                /* arp数据域 通过变长数组表示 */
#endif            
} __attribute__((packed))