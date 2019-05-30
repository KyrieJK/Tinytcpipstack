#include "netif.h"
#include "ethernet.h"
#include "arp.h"
#include "lib.h"

#define BROADCAST_ADDR ((unsigned char *)"\xff\xff\xff\xff\xff\xff")

/* arp请求,广播 */
void arp_request(struct arpentry *ae){
    struct pk_buff *pkb;
    struct ethernet_hdr *ehdr;
    struct arphdr *ahdr;

    pkb = alloc_pkb(ETH_HRD_SZ+ARP_HRD_SZ);
    ehdr = (struct ethernet_hdr *)pkb->pk_data;
    ahdr = (struct arphdr *)ehdr->eth_data;
    ahdr->arp_hrd = _htons(ARPHRD_ETHER);
    ahdr->arp_pro = _htons(ETHERNET_TYPE_IP);
    ahdr->arp_hlen = ETHERNET_ADDR_LEN;
    ahdr->arp_plen = IP_ALEN;
    ahdr->arp_op = _htons(ARPOP_REQUEST); /* arp请求 */
    /* IP地址均为网络字节序 */
    ahdr->arp_sip = ae->ae_dev->net_ipaddr;
    hwacpy(ahdr->arp_sha,ae->ae_dev->net_hwaddr);
    ahdr->arp_tip = ae->ae_ipaddr;
    hwacpy(ahdr->arp_tha,BROADCAST_ADDR);

    netdev_tx(ae->ae_dev,pkb,pkb->pk_len-ETH_HRD_SZ,ETHERNET_TYPE_ARP,BROADCAST_ADDR);
}

/* 接收数据包，进行层层拆包
   判断pk_type、pk_len是否合理
   比较arp包中的hardware address是否与eth包中的hardware address相同
   判断arp包中的L2、L3协议类型与长度，arp_op code
 */
void arp_in(struct net_device *dev,struct pk_buff *pkb){
    /* 层层拆包 */
    struct ethernet_hdr *ehdr = (struct ethernet_hdr *)pkb->pk_data;
    struct arphdr *ahdr = (struct arphdr *)ehdr->eth_data;
    if(pkb->pk_type == PKT_OTHERHOST){
        free_pkb(pkb);
    }

    if(pkb->pk_len < ETH_HRD_SZ+ARP_HRD_SZ){
        free_pkb(pkb);
    }

    if(hwacmp(ahdr->arp_sha,ehdr->eth_src) != 0){
        free_pkb(pkb);
    }

    arp_ntoh(ahdr);

#if defined(ARP_ETHERNET) && defined(ARP_IP)
    if(ahdr->arp_hrd != ARPHRD_ETHER || ahdr->arp_pro != ETHERNET_TYPE_IP ||
        ahdr->arp_hlen != ETHERNET_ADDR_LEN || ahdr->arp_plen != IP_ALEN){
            free_pkb(pkb);
        }
#endif    

    if(ahdr->arp_op != ARPOP_REQUEST && ahdr->arp_op != ARPOP_REPLY){
        free_pkb(pkb);
    }

    /* 经过以上判断，如果成功，则进行接收携带有arp协议的数据包 */
    arp_recv(dev,pkb);
    return;
}

void arp_recv(struct net_device *dev,struct pk_buff *pkb){
    struct ethernet_hdr *ehdr = (struct ethernet_hdr *)pkb->pk_data;
    struct aprhdr *ahdr = (struct arphdr *)ehdr->eth_data;
    struct arpentry *ae;

    
}
