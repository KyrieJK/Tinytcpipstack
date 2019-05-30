#include "netif.h"
#include "lib.h"

#define LOOPBACK_MTU 1500
#define LOOPBACK_IPADDR 0x7F000001 /* 127.0.0.1 */
#define LOOPBACK_NETMASK 0x000000FF /* 255.0.0.0 */

/**
 * 环回设备接口实现
 * **/

struct net_device *loop;

static int loop_dev_init(struct net_device *dev){
    dev->net_mtu=LOOPBACK_MTU;
    dev->net_ipaddr=LOOPBACK_IPADDR;
    dev->net_mask=LOOPBACK_NETMASK;
    return 0;
}

static void loop_recv(struct net_device *dev,struct pk_buff *pkb){
    dev->netdev_stats.rx_packets++;
    dev->netdev_stats.rx_bytes += pkb->pk_len;
    ethernet_in(dev,pkb);/* 判断数据包中的协议类型，然后根据协议类型选择handler函数 */
}

/**
 * 环回接口把数据包发送给本机环回接口
 * 然后将这个pkb传回给协议栈的环回接口接收函数处理
 * **/
static int loop_xmit(struct net_device *dev,struct pk_buff *pkb){
    get_pkb(pkb);
    loop_recv(dev,pkb);
    dev->netdev_stats.tx_packets++;
    dev->netdev_stats.tx_bytes+=pkb->pk_len;
    return pkb->pk_len;
}

static struct net_device_stats loopback_stats;

static struct net_device_stats *get_netdev_stats(struct net_device *dev){
    struct net_device_stats *stats = &loopback_stats;
    stats->rx_packets = dev->netdev_stats.rx_packets;
    stats->tx_packets = dev->netdev_stats.tx_packets;
    stats->rx_bytes = dev->netdev_stats.rx_bytes;
    stats->tx_bytes = dev->netdev_stats.tx_bytes;
    return stats;
}

void loop_init(void){
    loop = netdev_alloc("loopback",&loop_ops);
}

void loop_exit(void){
    netdev_free(loop);
}

/**
 * loopback设备接口操作函数集
 * **/
static struct netdev_ops loop_ops = {
    .init = loop_dev_init,
    .hard_xmit = loop_xmit,
    .get_netdev_stats = get_netdev_stats,
    .exit = loop_exit,
};

