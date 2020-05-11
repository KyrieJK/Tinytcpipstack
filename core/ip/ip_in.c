//
// Created by 张家望 on 2020/5/6.
//

#include "../../include/ip.h"
#include "../../include/lib.h"
#include "../../include/route.h"
#include "../../include/checksum.h"

class icmp_in;

void ip_recv_local(struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    /**
     * 判断packet buffer是否存在分片
     */
    if (iphdr->frag_off & (IP_FRAG_OFFSET | IP_FRAG_MF)) {
        if (iphdr->frag_off & IP_FRAG_DF){
            printf("error fragment");
            free_pkb(pkb);
            return;
        }
        pkb=ip_reass(pkb);
        if (pkb==NULL)
            return;
        iphdr=pkb2ip(pkb);
    }

    /*copy pkb to raw*/
    raw_in(pkb);

    /**
     * 解析IP头中的协议字段，传递至传输层中给指定协议接收
     */
    switch (iphdr->protocol) {
        case IP_P_ICMP:
            icmp_in(pkb);
            break;
        case IP_P_TCP:
            tcp_in(pkb);
            break;
        case IP_P_UDP:
            udp_in(pkb);
            break;
        default:
            free_pkb(pkb);
            printf("unknown protocol");
            break;
    }
}

/**
 * 数据报路由功能
 * @param pkb
 */
void ip_recv_route(struct pk_buff *pkb){
    if (rt_input(pkb)<0)
        return;
    /**
     * 判断packet buffer是否发往本地
     */
    if (pkb->pk_rtdst->rt_flags & RT_LOCALHOST){
        ip_recv_local(pkb);
    } else{
        ip_hton(pkb2ip(pkb));
        ip_forward(pkb);/*如果不是发往本地，则执行数据报转发*/
    }
}

/**
 * 接收IP数据报的入口函数
 * @param dev
 * @param pkb
 */
void ip_in(struct net_device *dev,struct pk_buff *pkb){
    struct ethernet_hdr *ehdr=(struct ethernet_hdr *)pkb->pk_data;
    struct ip *iphdr = (struct ip *)ehdr->eth_data;
    int hlen;

    if (pkb->pk_type==PKT_OTHERHOST){
        printf("IP数据报不是发往本地址的");
        free_pkb(pkb);
    }

    if (pkb->pk_len<ETH_HRD_SZ+IP_HRD_SZ){
        printf("IP数据报过小");
        free_pkb(pkb);
    }

    if (ipversion(iphdr)!=IP_VERSION_4){
        printf("IP数据报不是IPv4");
        free_pkb(pkb);
    }
    hlen=iphlen(iphdr);
    if (hlen<IP_HRD_SZ){
        printf("IP头过小");
        free_pkb(pkb);
    }

    if (ip_chksum((unsigned short *)iphdr,hlen) != 0){
        printf("IP校验和错误");
        free_pkb(pkb);
    }

    ip_ntoh(iphdr);
    if (iphdr->tot_len<hlen || pkb->pk_len < ETH_HRD_SZ+iphdr->tot_len){
        printf("IP数据报大小错误");
        free_pkb(pkb);
    }
    if (pkb->pk_len>ETH_HRD_SZ+iphdr->tot_len){
        pkb_trim(pkb,ETH_HRD_SZ+iphdr->tot_len);
    }

    ip_recv_route(pkb);
    return;
}
