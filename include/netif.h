//
// Created by KyrieJK on 2019/4/23.
//

#ifndef TINYTCPIPSTACK_NETIF_H
#define TINYTCPIPSTACK_NETIF_H

#include "list.h"

#define NETDEV_HWALEN 6
#define IFNAMESIZE 16

/**
 * 此结构用来描述网络设备
 * 在本项目中主要是用来描述tap设备
 */
struct net_device{
    int net_mtu;/*网络设备接口的最大传输单元,标识设备能处理数据帧的最大尺寸*/
    unsigned int net_ipaddr;/*网络设备绑定的IP地址*/
    unsigned int net_mask;/*网络设备的子网掩码*/
    unsigned char net_hwaddr[NETDEV_HWALEN];/*设备硬件地址*/
    unsigned char net_name[IFNAMESIZE];/* 设备名称 */
    struct list_head net_list;/* 网络设备链表 */
    struct netdev_ops *net_ops;/* 网络设备操作函数集 */
    struct net_device_stats netdev_stats;/* 网络设备接口统计信息 */
};

/* 网络设备操作函数集 */
struct netdev_ops {
    int (*hard_xmit)();/* 发送数据包 */
    int (*init)(struct netdev *);/* 网络设备初始化 */
    struct net_device_stats* (*get_netdev_stats)(struct net_device *);/*获取网络设备接口统计信息*/
    unsigned int (*localnet) (struct netdev *);/* 获取网络设备子网地址 */
    void (*exit)(struct netdev *);/* 网络设备注销 */
};

/*网络设备接口统计信息*/
struct net_device_stats{
    unsigned int rx_packets; /* 接收数据报总数 */
    unsigned int tx_packets; /* 发送数据包总数 */
    unsigned int rx_bytes;   /* 接收的总字节数 */
    unsigned int tx_bytes;   /* 发送的总字节数 */
    unsigned int rx_errors;  /* 接收的坏数据包 */
    unsigned int tx_errors;  /* 发送过程中出现错误的数据包 */
};

/* tap设备(虚拟网络设备)
 * 通过此设备程序可以很方便地模拟网络行为
 * 这是为了验证本项目所实现的tcp/ip 协议栈所使用的
 * unix系统中以一切都是文件为中心思想
 * 在Linux系统中实现tap设备的内核模块为tun，其模块介绍为Universal TUN/TAP device driver
 * 该模块提供了一个设备接口/dev/net/tun供用户层程序写入
 * 用户层程序通过读写/dev/net/tun来向主机内核协议栈注入数据或接收来自主机内核协议栈的数据
 * 可以把tun/tap看成数据管道，它一端连接主机协议栈，另一端连接用户程序
 * 为了使用tun/tap设备，用户层程序需要通过系统调用打开/dev/net/tun获得一个读写该设备的文件描述符(File Descriptor)
 * 并调用ioctl()向内核注册一个TUN或TAP类型的虚拟网卡(实例化一个tun/tap设备)，其名称可能是tap7b7ee9a9-c1/vnetXX/tunXX/tap0等
 * 此后用户程序可以通过该虚拟网卡与主机内核协议栈交互
 * 当用户程序关闭后，其注册的TUN或TAP虚拟网卡以及路由表相关条目都会被内核释放。
 * 可以把用户层程序看做是网络上另一台主机，他们通过tap虚拟网卡相连
 * TUN和TAP设备区别在于他们工作的协议栈层次不同，TAP等同于一个以太网设备
 * 用户层程序向TAP设备读写的是二层数据包如以太网数据帧，tap设备最常用的就是作为虚拟机网卡
 * TUN则模拟了网络层设备，操作第三层数据包比如IP数据包
 * */
struct tapdev{
    struct net_device dev;
    int fd;
};

/**
 * packet buffer
 * 用来描述已接收或者待发送的数据报文信息
 * pkb在不同网络协议层之间传递，可被用于不同网络协议
 * 如二层的以太网协议，三层的IP协议，四层的TCP或UDP协议
 * 其中某些成员变量会在该结构从一层向另一层传递时发生改变，从上层向下层传递需要添加首部
 * 从下层向上层传递需要移除首部
 */
struct pk_buff{
    struct list_head pk_list;/* 通过list_head双向循环链表结构的next和prev指针将多个pkb连接起来 */
    unsigned short pk_protocol;
    unsigned short pk_type;
    int pk_len;
    int pk_refcnt; /* 引用计数 */
    struct netdev *pk_indev;
    struct rt_entry *pk_rtdst; /* 路由目的入口 */
};

#endif //TINYTCPIPSTACK_NETIF_H
