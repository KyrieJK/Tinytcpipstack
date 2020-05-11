#include "../../include/netif.h"
#include "../../include/ethernet.h"
#include "../../include/arp.h"
#include "../../include/lib.h"

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

/* 回复arp请求 */
void arp_reply(struct net_device *dev,struct pk_buff *pkb){
    struct ethernet_hdr *ehdr = (struct ethernet_hdr *)pkb->pk_data;
    struct arphdr *ahdr = (struct arphdr *)ehdr->eth_data;
    ahdr->arp_op = ARPOP_REPLY;
    hwacpy(ahdr->arp_tha,ahdr->arp_sha);
    ahdr->arp_tip = ahdr->arp_sip;
    hwacpy(ahdr->arp_sha,dev->net_hwaddr);
    ahdr->arp_sip = dev->net_ipaddr;
    arp_hton(ahdr);/* 字节序转换 */

    netdev_tx(dev,pkb,ARP_HRD_SZ,ETHERNET_TYPE_ARP,ehdr->eth_src);
}

/* 解析pkb中的arphdr，与现有arp缓存表做对比 */
void arp_recv(struct net_device *dev,struct pk_buff *pkb){
    struct ethernet_hdr *ehdr = (struct ethernet_hdr *)pkb->pk_data;
    struct arphdr *ahdr = (struct arphdr *)ehdr->eth_data;
    struct arpentry *ae;

    if(ipv4_is_multicast(ahdr->arp_tip)){
        free_pkb(pkb);
    }

    if(ahdr->arp_tip != dev->net_ipaddr){
        free_pkb(pkb);
    }

    ae = arp_lookup(ahdr->arp_pro,ahdr->arp_sip);
    if(ae!=NULL){
        if(hwacmp(ae->ae_hwaddr,ahdr->arp_sha) != 0){
            ae->ae_state = ARP_PENDING;/* PENDING means a request for this entry has been sent,but the reply has not been received yet,用于等待arp_reply */
            arp_queue_send(ae); /* 所以需要再次retry 发送pending packet */
        }
        ae->ae_state = ARP_RESOLVED;
        ae->ae_ttl = ARP_TIMEOUT;
    }else if(ahdr->arp_op == ARPOP_REQUEST){ /* 如果判断entry为空，且arp_op 为request，则新插入一条ARP缓存记录 */
        arp_insert(dev,ahdr->arp_pro,ahdr->arp_sip,ahdr->arp_sha);
    }

    if(ahdr->arp_op == ARPOP_REQUEST){
        arp_reply(dev,pkb);
        return;
    }

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




