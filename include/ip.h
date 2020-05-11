#ifndef __IP_H
#define __IP_H

#include "netif.h"
#include "ethernet.h"
#include "list.h"

#define IP_ALEN 4 /* 单位是字节，即为4*8=32bit */
#define IP_VERSION_4 4

#define IP_FRAG_CE  0x8000 /*Flag:Congestion*/
#define IP_FRAG_DF  0x4000 /*Flag:Don't Fragment*/
#define IP_FRAG_MF  0x2000 /*Flag:More Fragment*/
#define IP_FRAG_OFFSET  0x1FFF /*Fragment offset part*/
#define IP_FRAG_MASK    (IP_FRAG_OFFSET | IP_FRAG_MF)

/**
 * 协议类型
 */
#define IP_P_IP        0
#define IP_P_ICMP    1
#define IP_P_IGMP    2
#define IP_P_TCP    6
#define IP_P_EGP    8
#define IP_P_UDP    17
#define IP_P_OSPF    89
#define IP_P_RAW    255
#define IP_P_MAX    256

/*
 * IP报文结构体
 * 我们定义默认情况下本地字节序为小端字节序
 */
struct ip {
    /*位域结构：这是在读kernel代码新学到的东西，平时很少用到这个结构。
     *有些信息在存储时，并不需要占用一个完整的字节，而只需占几个或一个二进制位。
     *Bit field为一种数据结构，可以把数据以位的形式紧凑的储存，并允许程序员对此结构的位进行操作
     *这种数据结构的好处是它可以使数据单元节省存储空间,当程序需要大数量级的数据单元时，使用这种数据结构就显得非常重要。
     * 第二个好处是位段可以很方便的访问一个整数值的部分从而可以化简源代码。
     * 因此我们可以看到linux kernel程序中大量使用了Bit field结构
     * 但是这种数据结构的缺点在于，Bit field实现依赖于具体的机器和系统，在不同的平台可能有不同的结果，这导致了Bit field在本质上不可移植的
     */
    unsigned char ihl: 4, //协议头长度，指的是首部占有的32位字数，该段占4位。因此最大值为15，以4字节为单位，因此IP头最大长度为60，由于基本报头长度为20字节，因此IP选项最多为40字节
    version: 4; //IP协议版本
    unsigned char tos; /*服务类型，此字段的值已经与标准协议有所区别。可以为流媒体相关协议所使用*/
    unsigned short tot_len; /*报文总长度，包括报头和分片。这里不包括链路层的头部，目前最大值是65535字节*/
    unsigned short id; /*IP标识。标识字段唯一标识主机发送的每一份数据报。通常每发送一份数据报，ID值就会递增1。来自同一个数据报的若干分片必须有一样的值*/
    unsigned short frag_off; /*分片在原始报文中的偏移*/
    unsigned char ttl; /*TTL生存时间，路由器在每次转发时递减此值*/
    unsigned char protocol; /*L4层协议标识*/
    unsigned short checksum; /*校验和*/
    unsigned int saddr; /*源地址*/
    unsigned int daddr; /*目的地址*/
    unsigned char data[0]; /*数据域data field*/
};

#define IP_HRD_SZ sizeof(struct ip)
#define ipversion(ip)   ((ip)->version)
#define iphlen(ip)  ((ip)->ihl*4)
#define ipdlen(ip)  ((ip)->tot_len-iphlen(iphdr))
#define ipdata(ip)  ((unsigned char *)(ip)+iphlen(ip))
#define pkb2ip(pkb) ((struct ip *)((pkb)->pk_data+ETH_HRD_SZ))

static inline void ip_ntoh(struct ip *iphdr) {
    iphdr->tot_len = _ntohs(iphdr->tot_len);
    iphdr->id = _ntohs(iphdr->id);
    iphdr->frag_off = _ntohs(iphdr->frag_off);
}

#define ip_hton(ip) ip_ntoh(ip)

/**
 * 通过子网掩码判断是否处于同一子网下
 * @param netmask
 * @param ip1
 * @param ip2
 * @return
 */
static inline int is_subnet(unsigned int netmask, unsigned int ip1, unsigned int ip2) {
    return ((netmask & ip1) == (netmask & ip2));
}

/**
 * 分片结构体
 */
struct fragment {
    unsigned short frag_id;
    unsigned int frag_src;
    unsigned int frag_dst;
    unsigned short frag_pro;
    unsigned int frag_hlen;
    unsigned int frag_rsize;/*已经到达分片数据大小*/
    unsigned int frag_size;/*期待到达的分片总大小*/
    int frag_ttl;/*分片重组计时器*/
    unsigned int frag_flags;/*分片标志位*/
    struct list_head frag_list;/*是一个链表，内容是目前已经接收的片段*/
    struct list_head frag_pkb;
};
/*所有片段都已经被接收，因此，可以连接起来获取原有IP包*/
#define FRAG_COMPLETE	0x00000001
/*片段中的第一个片段(offset=0的片段)已经接收到了。第一个片段就是唯一一个携带原有IP包中所有选项的片段*/
#define FRAG_FIRST_IN	0x00000002
/*片段中最后一个片段(MF=0的片段)已经接收到了。最后一个片段很重要，因为这个片段会告知我们原有IP包的尺寸*/
#define FRAG_LAST_IN	0x00000004
#define FRAG_FL_IN	0x00000006	/* first and last in*/

#define FRAG_TIME 30 /*IP片段不能永久存在内存中，而且当重组不可能时，一段时间后，就应该删除。该字段就是处理此事的定时器*/

#define ipv4_is_loopback(addr) ((addr & _htonl(0xff000000)) == _htonl(0x7f000000))
#define ipv4_is_multicast(addr) ((addr & _htonl(0xf0000000)) == _htonl(0xe0000000))
#define ipv4_is_broadcast(addr) (addr == _htonl(0xffffffff))
#define ipv4_is_zeronet(addr) ((addr & _htonl(0xff000000)) == _htonl(0x00000000)) /* 0.0.0.0表示本网络中的所有主机 */

#define IPFMT "%d.%d.%d.%d"
#define ipfmt(ip) (ip) & 0xff,((ip) >> 8) & 0xff,((ip) >> 16) & 0xff,((ip) >> 24) & 0xff

static inline int equalsubnet(unsigned int mask,unsigned int ip1,unsigned int ip2){
    return ((mask & ip1) == (mask & ip2));
}

struct pk_buff *ip_reass(struct pk_buff *);
void ip_send_frag(struct net_device *, struct pk_buff *);
void ip_send_dev(struct net_device *, struct pk_buff *);

#endif