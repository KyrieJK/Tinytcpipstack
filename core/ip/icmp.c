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
