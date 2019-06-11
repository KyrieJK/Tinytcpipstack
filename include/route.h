//
// Created by Kyrie on 2019/4/26.
//

#ifndef TINYTCPIPSTACK_ROUTE_H
#define TINYTCPIPSTACK_ROUTE_H

#include "netif.h"

/**
 * 路由目的入口结构体
 * 每一个实体相当于一个路由表项
 * 这样协议栈根据路由目的入口发送IP数据报
 */
struct rt_entry{
    struct list_head rt_list; /* 链接多个rt_entry的双向循环链表 */
    unsigned int rt_net; /* 网络地址 */
    unsigned int rt_netmask; /* 子网掩码 */
    unsigned int rt_gateway; /* 网关，数据报下一跳的目的地 */
    unsigned int rt_flags; /* 路由标志位 */
    int rt_metric; /* 路由度量值 */
    struct netdev *rt_dev; /* 相关网络设备 */
};

#define RT_LOCALHOST 0x00000000 /* 本机地址 loopback */
#define RT_LOCALNET 0x00000001 /* 本地子网地址 */
#define RT_DEFAULT 0x00000002 /* 默认网关 */


#endif //TINYTCPIPSTACK_ROUTE_H
