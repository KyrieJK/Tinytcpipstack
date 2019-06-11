//
// Created by Kyrie on 2019/4/28.
//

#ifndef TINYTCPIPSTACK_ETHERNET_H
#define TINYTCPIPSTACK_ETHERNET_H

#include <string.h>

#define ETH_HRD_SZ sizeof(struct ethernet_hdr) /* 以太网首部长度 */
#define ETHERNET_ADDR_LEN 6 /* 以太网地址长度 */

#define ETHERNET_TYPE_IP 0x0800
#define ETHERNET_TYPE_ARP 0X0806
#define ETHERNET_TYPE_RARP 0X8035
#define ETHERNET_TYPE_ALL 0x3

struct ethernet_hdr{
    unsigned char eth_dst[ETHERNET_ADDR_LEN];
    unsigned char eth_src[ETHERNET_ADDR_LEN];
    unsigned short protocol; /* 数据报文类型 */
    unsigned char eth_data[0];/* 变长数组，在内存地址中与结构体ethernet_hdr连续 */
};

static inline void hwacpy(void *dst,void *src){
    memcpy(dst,src,ETHERNET_ADDR_LEN);
}

static inline void hwaset(void *dst,int val){
    memset(dst,val,ETHERNET_ADDR_LEN);
}

static inline int hwacmp(void *hwa1,void *hwa2){
    return memcmp(hwa1,hwa2,ETHERNET_ADDR_LEN);
}

#define mac_addr(hwa) (hwa)[0],(hwa)[1],(hwa)[2],(hwa)[3],(hwa)[4],(hwa)[5]
#define mac_to_p "%02x:%02x:%02x:%02x:%02x:%02x"

static inline char *eth_type(unsigned short type){
    if(type == ETHERNET_TYPE_IP)
        return "IP";
    else if (type==ETHERNET_TYPE_ARP)
        return "ARP";
    else if(type==ETHERNET_TYPE_RARP)
        return "RARP";
    else if(type==ETHERNET_TYPE_ALL)
        return "ALL";
    else
        return "unknown";
}

/**
 * 组播MAC地址的第一个字节为0x01
 * @param hwa
 * @return
 */
static inline int is_ethernet_multicast(unsigned char *hwa){
    return (hwa[0] & 0x01);
}
/**
 * 广播是组播的特例，表示所有地址，用全F表示：FF-FF-FF-FF-FF-FF
 * @param hwa
 * @return
 */
static inline int is_ethernet_broadcast(unsigned char *hwa){
    return (hwa[0] & hwa[1] & hwa[2] & hwa[3] & hwa[4] & hwa[5]) == 0xff;
}


#endif //TINYTCPIPSTACK_ETHERNET_H
