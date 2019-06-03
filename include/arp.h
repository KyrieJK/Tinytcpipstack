#ifndef __ARP_H
#define __ARP_H

#include "ethernet.h"
#include "list.h"
#include "netif.h"
#include "ip.h"

/* 定义L2、L3 协议类型 */
#define ARP_ETHERNET
#define ARP_IP

/* ARP高速缓存表项容量 */
#define ARP_CACHE_SZ 20
#define ARP_TIMEOUT 600 /* 缓存项过期时间 */
#define ARP_WAITTIME 1 /* arp等待时间 */

/* ARP高速缓存表项状态 */
#define ARP_FREE 1
#define ARP_PENDING 2
#define ARP_RESOLVED 3

#define ARP_REQ_ATTEMPT 4

/* 硬件地址类型 */
#define ARPHRD_ETHER 1
/* ARP协议操作码(参考linux kernel中的定义) */
#define ARPOP_REQUEST         1     /* ARP request */
#define ARPOP_REPLY           2     /* ARP reply */
#define ARPOP_RREQUEST        3     /* RARP request */ 
#define ARPOP_RREPLY          4     /* RARP reply */
#define ARPOP_INREQUEST       8     /* InARP request */
#define ARPOP_INREPLY         9     /* InARP reply */

#define ARP_HRD_SZ sizeof(struct arphdr)

struct arphdr{
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
} __attribute__((packed));

/* 主机字节序序转换为网络字节序 */
static inline void arp_hton(struct arphdr *ahdr){
    ahdr->arp_hrd = _htons(ahdr->arp_hrd);
    ahdr->arp_pro = _htons(ahdr->arp_pro);
    ahdr->arp_op = _htons(ahdr->arp_op);
}

#define arp_ntoh(ahdr) arp_hton(ahdr)

/* arp高速缓存表项 */
struct arpentry{
    struct list_head ae_list; /* pending queue for resolved hardware address */
    struct net_device *ae_dev; /* 关联的网络接口 */
    int ae_retry; /* arp请求重复次数 */
    int ae_ttl; /* arp entry timeout */
    unsigned int ae_state; /* arp entry state */
    unsigned short ae_pro; /* L3 protocol */
    unsigned int ae_ipaddr; /* L3 protocol address */
    unsigned char ae_hwaddr[ETHERNET_ADDR_LEN]; /* L2 protocol address(ethernet) */
};

extern void arp_request(struct arpentry *ae);

#endif
