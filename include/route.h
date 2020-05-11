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
    unsigned int rt_flags; /* 路由标志位，用于路由表项的一些特性和标志 */
    int rt_metric; /* 距离度量值 */
    struct net_device *rt_dev; /* 输出网络设备（即将报文送达目的地的发送设备），对送往本地的输入报文的路由，输出网络设备设置为回环设备 */
};

#define RT_NONE 0x00000000 /* 无标志类型 */
#define RT_LOCALHOST 0x00000001 /* 本地子网地址 */
#define RT_DEFAULT 0x00000002 /* 默认网关 */

extern int rt_output(struct pk_buff *);
extern void rt_init(void);
extern void rt_add(unsigned int, unsigned int, unsigned int, int, unsigned int,
                   struct net_device *);
extern int rt_input(struct pk_buff *);


#endif //TINYTCPIPSTACK_ROUTE_H
