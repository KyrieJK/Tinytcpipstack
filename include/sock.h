//
// Created by JKerving on 2019/4/27.
//

#ifndef TINYTCPIPSTACK_SOCK_H
#define TINYTCPIPSTACK_SOCK_H

#include "netif.h"
#include "list.h"

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
    struct list_head recv_queue;
    unsigned int hash; /* 哈希表，存储元素为sock结构体，便于索引 */
    struct hlist_node hash_list;
    int refcnt; /* 引用计数 */
};

/* sock结构体操作函数集 */
struct sock_ops{
    int (*hash)(struct sock *);
    void (*unhash)(struct sock *);
    int (*bind)(struct sock *, struct sock_addr *);
    int (*connect)(struct sock *, struct sock_addr *);
    int (*listen)(struct sock *,int);
    struct sock *(*accept)(struct sock *);
    int (*close)(struct sock *);
    int (*set_port)(struct sock*, unsigned short);
    struct pk_buff *(*recv_pkb)(struct sock *);
    int (*recv_buf)(struct sock *,char *,int);
    int (*send_pkb)(struct sock *, struct pk_buff *);
    int (*send_buf)(struct sock *,void *,int , struct sock_addr *);
};

#define hlist_for_each_sock(sk,node,head)\
    hlist_for_each_entry(sk,node,head,hash_list)

#endif //TINYTCPIPSTACK_SOCK_H
