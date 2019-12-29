//
// Created by Kyrie on 2019/4/28.
//

#ifndef TINYTCPIPSTACK_INET_H
#define TINYTCPIPSTACK_INET_H

#include "socket.h"

/*
 * 本项目中仅支持Internet协议族套接字
 * 因此仅实现了INET套接字种类
 * */
struct inet_type{
    struct sock *(alloc_sock)(int);/* sock结构分配函数 */
    int type; /*套接字类型*/
    int protocol;/* 传输协议 */
};

struct socket_ops inet_ops;/* INET类型套接字操作函数集 */
void inet_init(void);/* INET类型套接字初始化函数 */

#endif //TINYTCPIPSTACK_INET_H
