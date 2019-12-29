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

#include "../../include/socket.h"
#include "../../include/sock.h"
#include "../../include/lib.h"
#include "../../include/list.h"

int alloc_socks = 0;
int free_socks = 0;

struct sock *get_sock(struct sock *sk) {
    sk->refcnt++;
    return sk;
}

void free_sock(struct sock *sk) {
    if (--sk->refcnt <= 0) {
        free_socks++;
        free(sk);
    }
}

/* 数据包接收通知 */
void sock_recv_notify(struct sock *sk){
    if (!list_empty(&sk->recv_queue)&&sk->recv_wait){
        wake_up(sk->recv_wait);
    }
}

/* 按顺序取出接收队列中的pk_buff，每次函数调用取首个pk_buff */
struct pk_buff* sock_recv_pkbuff(struct sock *sk){
    struct pk_buff *pkb = NULL;
    while (1){
        if (!list_empty(&sk->recv_queue)){
            pkb = list_first_entry(&sk->recv_queue, struct pk_buff,pk_list);
            list_del_init(&pkb->pk_list);
            break;
        }
        if (sleep_on(sk->recv_wait)<0) /* 等待队列阻塞，通过等待某个pthread_cond_signal通知唤醒 */
            break;
    }
    return pkb;
}

void sock_add_hash(struct sock *sk, struct hlist_head *head) {
    get_sock(sk);
    hlist_add_head(&sk->hash_list, head);
}

void sock_del_hash(struct sock *sk) {
    if (!hlist_unhashed(&sk->hash_list)) {
        hlist_del(&sk->hash_list);
        free_sock(sk);
    }
}

int sock_close(struct sock *sk) {
    struct pk_buff *pkb;
    sk->recv_wait = NULL;
    /* 从hlist中删除对应的sk实例的hash信息，停止接收信息 */
    if (sk->ops->unhash)
        sk->ops->unhash(sk);
    /* 清空sk实例等待队列 */
    while (!list_empty(&sk->recv_queue)) {
        pkb = list_first_entry(&sk->recv_queue, struct pk_buff, pk_list);
        list_del(&pkb->pk_list);
        free_pkb(pkb);
    }
    return 0;
}

int sock_autobind(struct sock *sk){
    if (sk->ops->set_port)
        return sk->ops->set_port(sk,0);
    return -1;
}