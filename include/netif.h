//
// Created by KyrieJK on 2019/4/23.
//

#ifndef TINYTCPIPSTACK_NETIF_H
#define TINYTCPIPSTACK_NETIF_H

#define NETDEV_HWALEN 6
#define IFNAMESIZE 16

/**
 * 此结构用来描述网络设备
 * 在本项目中主要是用来描述tap设备
 */
struct net_device{
    int net_mtu;//网络设备接口的最大传输单元,标识设备能处理数据帧的最大尺寸
    unsigned int net_ipaddr;//网络设备绑定的IP地址
    unsigned int net_mask;//网络设备的子网掩码
    unsigned char net_hwaddr[NETDEV_HWALEN];//设备硬件地址
    unsigned char net_name[IFNAMESIZE];//设备名称

};

#endif //TINYTCPIPSTACK_NETIF_H
