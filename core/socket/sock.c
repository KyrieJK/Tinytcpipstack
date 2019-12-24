/**
 * 传输控制块实现
 * 各协议族传输层使用各自的传输控制块存放套接口所要求的信息
 * sock结构是比较通用的网络层描述块，构成传输控制块的基础，与具体的协议族无关
 * sock结构描述了各协议族传输层协议的公共信息，因此不能直接作为传输层控制块来使用
 * 不同协议族的传输层在使用该结构时都会对其进行扩展
 * inet_sock结构时比较通用的IPv4协议族描述块，包含IPv4协议族基础传输层，即UDP、TCP以及原始传输控制块共有的信息
 * 
 * 注意：Tinytcpip协议栈中仅支持IPv4协议族，因此sock.h/sock.c 为通用的IPv4协议族描述块
 * 
 * 实现传输层通用函数
 */

#include "socket.h"
#include "sock.h"
#include "lib.h"
#include "list.h"
#include "../../include/list.h"

int free_socks = 0;

struct sock *get_sock(struct sock *sk){
    sk->refcnt++;
    return sk;
}

void free_sock(struct sock *sk){
    if(--sk->refcnt<=0){
        free_socks++;
        free(sk);
    }
}

void sock_add_hash(struct sock *sk,struct hlist_head *head){
    get_sock(sk);
    hlist_add_head(&sk->hash_list,head);
}

void sock_del_hash(struct sock *sk){
    if(!hlist_unhashed(&sk->hash_list)){
        hlist_del(&sk->hash_list);
        free_sock(sk);
    }
}

int sock_close(struct sock *sk){
    struct pk_buff *pkb;
    sk->recv_wait = NULL;
    if(sk->ops->unhash)
        sk->ops->unhash(sk);
    while(!list_empty(&sk->recv_queue)){
        pkb = list_first_entry(&sk->recv_queue,struct pk_buff,pk_list);
        list_del(&pkb->pk_list);
        free_pkb(pkb);
    }
    return 0;
}