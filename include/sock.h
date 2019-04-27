//
// Created by JKerving on 2019/4/27.
//

#ifndef TINYTCPIPSTACK_SOCK_H
#define TINYTCPIPSTACK_SOCK_H

struct sock_addr{
    unsigned int src_addr;
    unsigned int dst_addr;
    unsigned short src_port;
    unsigned short dst_port;
};

/* INET协议族的套接字，INET套接字就是支持Internet协议族的套接字，它的位于TCP协议之上，BSD套接字之下
 * Linux将不同的协议族套接字做抽象之后才形成了BSD套接字,也就是socket
 * 文件系统关系密切的部分放在socket结构中
 * 与通信关系密切的部分放在sock结构中
 * socket和sock是描述同一个事物的两个侧面
 * */
struct sock{
    unsigned char protocol; /* 传输层协议 */
    struct sock_addr sk_addr;/* 记录套接字绑定的地址与端口 */
    struct socket *sock;
    struct sock_ops *ops; /* sock结构体的操作函数集 */
    struct rtentry *sk_dst; /* 路由目的入口(next-hop) */
};

struct sock_ops{

};

#endif //TINYTCPIPSTACK_SOCK_H
