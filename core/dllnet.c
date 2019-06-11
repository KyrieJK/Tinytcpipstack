//
// Created by Kyrie on 2019/5/5.
//

#include <net/if.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/if_tun.h>
#include "../include/netif.h"
#include "../include/ethernet.h"
#include "../include/lib.h"
#include "../include/netconfig.h"

/**
 * ethernet header协议头提取。
 * 从接收的pkb->pk_data中提取ethernet header头部结构体
 * 通过判断ethdr的目的地址判断多播、广播还是单播
 * @param dev
 * @param pkb
 * @return
 */
static struct ethernet_hdr *eth_init(struct net_device *dev, struct pk_buff *pkb){
    struct ethernet_hdr *ethdr = (struct ethernet_hdr *)pkb->pk_data;
    if (pkb->pk_len<ETH_HRD_SZ) {
        free_pkb(pkb);
        dbg("recv packets too short:%d bytes",pkb->pk_len);
        return NULL;
    }
    if (is_ethernet_multicast(ethdr->eth_dst)) {
        if (is_ethernet_broadcast(ethdr->eth_dst)) {
            pkb->pk_type = PKT_BROADCAST;
        } else{
            pkb->pk_type = PKT_MULTICAST;
        }
    } else if(!hwacmp(ethdr->eth_dst,dev->net_hwaddr)) {/* 比较数据报文中的目的地址是否为本设备的MAC地址 */
        pkb->pk_type = PKT_LOCALHOST;
    } else{
        pkb->pk_type = PKT_OTHERHOST;
    }
    pkb->pk_protocol = _ntohs(ethdr->protocol); /* 数据帧中的数据报文类型 */
    return ethdr;
}

/**
 * 从接收到的pkb中取出ethernet header结构
 * 通过判断pkb中的报文协议类型字段，从而分发到对应的handler函数去处理pkb数据包
 * 如果是IP协议数据报文，则将pkb传递给IP协议处理
 * 如果是ARP协议数据报文，将pkb传递给ARP协议处理
 * @param dev
 * @param pkb
 */
void ethernet_in(struct net_device *dev, struct pk_buff *pkb){
    struct ethernet *ethdr = eth_init(dev,pkb);
    if(ethdr==NULL){
        return;
    }
    pkb->pk_indev = dev;
    switch (pkb->pk_protocol){
        case ETHERNET_TYPE_ARP:
            /* arp报文处理函数 */
            break;
        case ETHERNET_TYPE_IP:
            /* IP报文处理函数 */
            break;
        default:
            dbg("drop unknow type packet");
            free_pkb(pkb); /* 判断不出来的报文类型，直接丢失pkb包 */
            break;
    }
}