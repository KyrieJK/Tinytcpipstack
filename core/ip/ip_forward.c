//
// Created by 张家望 on 2020/5/6.
//

#include "../../include/ip.h"
#include "../../include/lib.h"
#include "../../include/netif.h"
#include "../../include/list.h"
#include "../../include/route.h"
#include "../../include/ethernet.h"
#include "../../include/checksum.h"
#include "../../include/icmp.h"


void ip_forward(struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct rt_entry *rt = pkb->pk_rtdst;
    struct net_device *indev = pkb->pk_indev;
    unsigned int dest;
#ifdef CONFIG_TOP1
    printf("不支持IP数据报转发");
    free_pkb(pkb);
#endif
    if (iphdr->ttl <= 1) {
        icmp_send(ICMP_T_TIMEEXCEED, ICMP_EXC_TTL, 0, pkb);
        free_pkb(pkb);
    }

    /*更新ttl字段*/
    iphdr->ttl--;
    ip_chksum((unsigned short *) iphdr, iphlen(iphdr));

    if ((rt->rt_flags & RT_DEFAULT) || rt->rt_metric > 0) {
        dest = rt->rt_gateway;
    } else {
        dest = iphdr->daddr;
    }

    if (indev == rt->rt_dev) {
        struct rt_entry *srt = rt_lookup(iphdr->saddr);
        if (srt && srt->rt_metric == 0 && equalsubnet(srt->rt_netmask,iphdr->saddr,dest)){
            icmp_send(ICMP_T_REDIRECT,ICMP_REDIR_HOST,dest,pkb);
        }
    }

    if (_ntohs(iphdr->tot_len) > rt->rt_dev->net_mtu){
        if (iphdr->frag_off & _htons(IP_FRAG_DF)){
            icmp_send(ICMP_T_DESTUNREACH,ICMP_FRAG_NEEDED,0,pkb);
            free_pkb(pkb);
        }
        ip_send_frag(rt->rt_dev,pkb);
    } else{
        ip_send_dev(rt->rt_dev,pkb);
    }
}
