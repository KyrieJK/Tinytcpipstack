//
// Created by 张家望 on 2020/5/4.
//
#include "../../include/ip.h"
#include "../../include/route.h"
#include "../../include/netif.h"
#include "../../include/icmp.h"
#include "../../include/lib.h"
#include "../../include/checksum.h"

static struct icmp_control icmp_table[ICMP_T_MAXNUM + 1] = {
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
static void icmp_dest_unreach(struct icmp_control *icmp_control, struct pk_buff *pkb) {
    ferr("destination unreachable");
    free_pkb(pkb);
}


static const char *redirectstr[4] = {
        [ICMP_REDIR_NET]="net redirect",
        [ICMP_REDIR_HOST]="host redirect",
        [ICMP_REDIR_NETTOS]="type of service and net redirect",
        [ICMP_REDIR_HOSTTOS]="type of service and host redirect"
};

static void icmp_redirect(struct icmp_control *icmp_control, struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct icmphdr *icmphdr = ip2icmp(iphdr);
    if (icmphdr->icmp_code > 4) {
        ferr("Redirect code %d is error", icmphdr->icmp_code);
    } else {
        ferr("from " IPFMT " %s(new nexthop "IPFMT")", ipfmt(iphdr->saddr), redirectstr[icmphdr->icmp_code],
             ipfmt(icmphdr->icmp_gw));
    }
    free_pkb(pkb);
}

/**
 * 由ICMP协议使用，回复需要做响应的入ICMP请求消息
 * @param icmp_control
 * @param pkb
 */
static void icmp_echo_reply(struct icmp_control *icmp_control, struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct icmphdr *icmphdr = ip2icmp(iphdr);
    ferr("from "IPFMT" id %d seq %d ttl %d", ipfmt(iphdr->saddr), _ntohs(icmphdr->icmp_un.echo.id),
         _ntohs(icmphdr->icmp_un.echo.seq)), iphdr->ttl;
    free_pkb(pkb);
}

static void icmp_echo_request(struct icmp_control *icmp_control, struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct icmphdr *icmphdr = ip2icmp(iphdr);
    ferr("echo request data %d bytes icmp_id %d icmp_seq %d", (int) (iphdr->tot_len - iphlen(iphdr) - ICMP_HRD_SZ),
         _ntohs(icmphdr->icmp_un.echo.id), _ntohs(icmphdr->icmp_un.echo.seq));
    if (icmphdr->icmp_code) {
        ferr("echo request packet corrupted");
        free_pkb(pkb);
        return;
    }
    icmphdr->icmp_type = ICMP_T_ECHORLY;
    iphdr->daddr = iphdr->saddr;
    ip_hton(iphdr);
    pkb->pk_rtdst = NULL;
    pkb->pk_indev = NULL;
    pkb->pk_type = PKT_NONE;
    ip_send_out(pkb);
}


static void icmp_drop_reply(struct icmp_control *icmp_control, struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct icmphdr *icmphdr = ip2icmp(iphdr);
    ferr("icmp type %d code %d (droppped)", icmphdr->icmp_type, icmphdr->icmp_code);
    if (icmp_control->info) {
        ferr("%s", icmp_control->info);
    }
    free_pkb(pkb);
}

void icmp_in(struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct icmphdr *icmphdr = ip2icmp(iphdr);
    int icmplen;
    int icmptype;
    icmplen = ipdlen(iphdr);
    ferr("%d bytes", icmplen);
    if (icmplen < ICMP_HRD_SZ) {
        ferr("ICMP Header长度发生错误");
        free_pkb(pkb);/*直接丢弃pkb*/
    }
    if (icmp_chksum((unsigned short *) icmphdr, icmplen) != 0) {
        ferr("ICMP Header 首部校验和错误");
        free_pkb(pkb);
    }
    icmptype=icmphdr->icmp_type;
    if (icmptype>ICMP_T_MAXNUM){
        ferr("ICMP type 无法识别");
        free_pkb(pkb);
    }
    icmp_table[icmptype].handler(&icmp_table[icmptype],pkb);
}

void icmp_send(unsigned char type,unsigned char code,unsigned int data,struct pk_buff *pkb_in){
    struct pk_buff *pkb;
    struct ip *iphdr = pkb2ip(pkb_in);
    struct icmphdr *icmphdr;
    int payloadLen = _ntohs(iphdr->tot_len);

}
