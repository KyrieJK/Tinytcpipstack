//
// Created by 张家望 on 2020/5/6.
//

#include "../../include/netif.h"
#include "../../include/ethernet.h"
#include "../../include/arp.h"
#include "../../include/ip.h"
#include "../../include/route.h"
#include "../../include/lib.h"
#include "../../include/icmp.h"
#include "../../include/checksum.h"

static unsigned short ip_id = 0;

/**
 * IP数据报发送函数
 * @param dev
 * @param pkb
 */
void ip_send_dev(struct net_device *dev, struct pk_buff *pkb) {
    struct arpentry *ae;
    unsigned int dest;
    struct rt_entry *rt = pkb->pk_rtdst;

    if (rt->rt_flags & RT_LOCALHOST) {
        printf("发往本地");
        netdev_tx(dev, pkb, pkb->pk_len - ETH_HRD_SZ, ETHERNET_TYPE_IP, dev->net_hwaddr);
        return;
    }

    if ((rt->rt_flags & RT_DEFAULT) || rt->rt_metric > 0)
        dest = rt->rt_gateway;/* 选择默认网关作为destination */
    else
        dest = pkb2ip(pkb)->daddr;

    ae=arp_lookup(ETHERNET_TYPE_IP,dest);
    if (!ae){
        ae = arp_alloc();
        if (!ae){
            printf("arp缓存已满");
            free_pkb(pkb);
            return;
        }
        ae->ae_ipaddr=dest;
        ae->ae_dev=dev;
        list_add_tail(&pkb->pk_list,&ae->ae_list);
        arp_request(ae);
    } else if (ae->ae_state==ARP_PENDING){
        printf("arp缓存项 waiting");
        list_add_tail(&pkb->pk_list,&ae->ae_list);
    } else{
        netdev_tx(dev,pkb,pkb->pk_len-ETH_HRD_SZ,ETHERNET_TYPE_IP,ae->ae_hwaddr);
    }
}

void ip_send_out(struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    pkb->pk_protocol = ETHERNET_TYPE_IP;
    if (!pkb->pk_rtdst && rt_output(pkb) < 0) {
        free_pkb(pkb);
        return;
    }
    ip_chksum((unsigned short *) iphdr, iphlen(iphdr));

    if (_ntohs(iphdr->tot_len) > pkb->pk_rtdst->rt_dev->net_mtu)
        ip_send_frag(pkb->pk_rtdst->rt_dev, pkb);
    else
        ip_send_dev(pkb->pk_rtdst->rt_dev, pkb);
}

/**
 * 封装为IP数据报发送
 * @param pkb
 * @param tos
 * @param len
 * @param ttl
 * @param protocol
 * @param dest
 */
void ip_send_info(struct pk_buff *pkb, unsigned char tos, unsigned short len, unsigned char ttl, unsigned char protocol,
                  unsigned int dest) {
    struct ip *iphdr = pkb2ip(pkb);
    /**
     * 填充IP头
     */
    iphdr->version = IP_VERSION_4;
    iphdr->ihl = IP_HRD_SZ / 4;
    iphdr->tos = tos;
    iphdr->tot_len = _htons(len);
    iphdr->id = _htons(ip_id++);
    iphdr->frag_off = 0;
    iphdr->ttl = ttl;
    iphdr->protocol = protocol;
    iphdr->daddr = dest;

    ip_send_out(pkb);
}

