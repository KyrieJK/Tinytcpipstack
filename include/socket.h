//
// Created by Kyrie on 2019/4/27.
//

#ifndef TINYTCPIPSTACK_SOCKET_H
#define TINYTCPIPSTACK_SOCKET_H

typedef enum{
    SS_FREE = 1,
    SS_UNCONNECTED,
    SS_BIND,
    SS_LISTEN,
    SS_CONNECTING,
    SS_CONNECTED,
    SS_MAX
} socket_state;
/*套接字类型分别为流、数据报、原始套接字，MAX用来判断type类型是否合法*/
enum sock_type{
    SOCK_STREAM = 1,
    SOCK_DGRAM,
    SOCK_RAW,
    SOCK_MAX
};
/* 本项目仅支持INET协议族 */
enum socket_family{
    AF_INET = 1
};

/* BSD套接字结构，为sock结构的更高级的抽象。是网络编程的接口，旨在为应用层提供大量网络调用接口 
   为了体现“一切皆文件”的理念，Linux也允许标准I/O系统调用通过一个套接口文件描述符来读写其对应套接口上的网络链接
   就像通过文件描述符
*/
struct socket{
    socket_state state; /* socket当前状态 */
    unsigned int family; /* 支持的协议族，在本项目中是用来实现支持INET协议族 */
    unsigned int type; /* socket在特定协议族下支持的套接字类型 */
    struct sock *sk; /* 指向sock结构的指针 */
    struct socket_ops *ops;
    int refcnt; /* 文件引用数，因为socket在linux系统中是文件形式 */
};


/* BSD套接字的操作函数集 */
struct socket_ops {
    int (*socket)(struct socket *,int);
    int (*close)(struct socket *);
    int (*accept)(struct socket *, struct socket *, struct sock_addr *);
    int (*listen)(struct socket *,int);
    int (*bind)(struct socket *, struct sock_addr *);
    int (*connect)(struct socket *, struct sock_addr *);
    int (*read)(struct socket *,void *,int);
    int (*write)(struct socket *,void *,int);
    int (*send)(struct socket *,void *,int, struct sock_addr *);
    struct pk_buff *(*recv)(struct socket *);
};

#endif //TINYTCPIPSTACK_SOCKET_H
