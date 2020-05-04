//
// Created by 张家望 on 2020/5/4.
//
#include "../../include/ip.h"
#include "../../include/route.h"
#include "../../include/netif.h"
#include "../../include/icmp.h"
#include "../../include/lib.h"

static struct icmp_control icmp_table[ICMP_T_MAXNUM+1]={
        [ICMP_T_ECHORLY]={
                .error=0,
                .handler=icmp_echo_reply,
        },
        [ICMP_T_DUMMY_1]=ICMP_DESC_DUMMY_ENTRY,
        [ICMP_T_DUMMY_2]=ICMP_DESC_DUMMY_ENTRY,
        [ICMP_T_DESTUNREACH]={
                .error=1,
                .handler=icmp_dest_unreach,
        },
        [ICMP_T_SOURCEQUENCH]={
                .error=1,
                .info="icmp source quench",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_REDIRECT]={
                .error=1,
                .info="icmp redirect",
                .handler=icmp_redirect,
        },
        [ICMP_T_DUMMY_6]=ICMP_DESC_DUMMY_ENTRY,
        [ICMP_T_DUMMY_7]=ICMP_DESC_DUMMY_ENTRY,
        [ICMP_T_ECHOREQ]={
                .error=0,
                .handler=icmp_echo_request,
        },
        [ICMP_T_DUMMY_9]=ICMP_DESC_DUMMY_ENTRY,
        [ICMP_T_DUMMY_10]=ICMP_DESC_DUMMY_ENTRY,
        [ICMP_T_TIMEEXCEED]={
                .error=1,
                .info="icmp time exceeded",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_PARAMPROBLEM]={
                .error=1,
                .info="icmp parameter problem",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_TIMESTAMPREQ]={
                .error=0,
                .info="icmp timestamp request",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_TIMESTAMPRLY]={
                .error=0,
                .info="icmp timestamp reply",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_INFOREQ]={
                .error=0,
                .info="icmp information request",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_INFORLY]={
                .error=0,
                .info="icmp information reply",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_ADDRMASKREQ]={
                .error=0,
                .info="icmp address mask request",
                .handler=icmp_drop_reply,
        },
        [ICMP_T_ADDRMASKRLY]={
                .error=0,
                .info="icmp address mask reply",
                .handler=icmp_drop_reply,
        }
};

/**
 * 处理目的主机不可达ICMP消息
 * 源码中处理比较复杂，此处我只输出错误信息提示，直接丢弃pkb包。
 * @param icmp_control
 * @param pkb
 */
static void icmp_dest_unreach(struct icmp_control *icmp_control,struct pk_buff *pkb){
    ferr("destination unreachable");
    free_pkb(pkb);
}


static const char *redirectstr[4]={
        [ICMP_REDIR_NET]="net redirect",
        [ICMP_REDIR_HOST]="host redirect",
        [ICMP_REDIR_NETTOS]="type of service and net redirect",
        [ICMP_REDIR_HOSTTOS]="type of service and host redirect"
};

static void icmp_redirect(struct icmp_control *icmp_control,struct pk_buff *pkb){
    struct ip *iphdr=pkb2ip(pkb);
    struct icmphdr *icmphdr=ip2icmp(iphdr);
    if (icmphdr->icmp_code>4){
        ferr("Redirect code %d is error",icmphdr->icmp_code);
    } else{
        ferr("from " IPFMT " %s(new nexthop "IPFMT")", ipfmt(iphdr->saddr),redirectstr[icmphdr->icmp_code],ipfmt(icmphdr->icmp_gw));
    }
    free_pkb(pkb);
}
