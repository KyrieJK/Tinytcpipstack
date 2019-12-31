/**
 * 针对IPv4协议族，套接口相关系统调用的套接口层实现
 */


#include "../../include/inet.h"
#include "../../include/netif.h"
#include "../../include/sock.h"
#include "../../include/socket.h"
#include "../../include/lib.h"
#include "../../include/route.h"

static struct inet_type inet_type_table[SOCK_MAX] = {
        [0]={},
        [SOCK_STREAM]={
                .type=SOCK_STREAM,
                .protocol=IP_P_TCP,
                .alloc_sock=tcp_alloc_sock,
        },
        [SOCK_DGRAM]={
                .type=SOCK_DGRAM,
                .protocol=IP_P_UDP,
                .alloc_sock=udp_alloc_sock,
        },
        [SOCK_RAW]={
                .type=SOCK_RAW,
                .protocol=IP_P_IP,
                .alloc_sock=raw_alloc_sock,
        }
};

static int inet_create(struct socket *sock, int protocol) {
    struct inet_type *inet;
    struct sock *sk;
    unsigned int type;

    sock->state = SS_UNCONNECTED; /* 初始化套接口为UNCONNECTED状态 */

    type = sock->type;
    if (type >= SOCK_MAX) {
        return -1;
    }

    inet = &inet_type_table[type]; /* 以套接口类型为索引选择对应的inet实例 */
    sk = inet->alloc_sock(protocol); /* 根据具体的传输层协议分配具体的sock传输控制块 */
    if (!sk) {
        return -1;
    }
    if (!protocol) {
        protocol = inet->protocol;
    }
    sock->sk = get_sock(sk);
    list_init(&sk->recv_queue);
    hlist_node_init(&sk->hash_list);
    sk->protocol = protocol;
    sk->sock = sock;

    if (sk->hash && sk->ops->hash) {
        sk->ops->hash(sk);
    }
    return 0;
}

static int inet_bind(struct socket *sock, struct sock_addr *skaddr) {
    struct sock *sk = sock->sk;
    int err = -1;
    /* 如果当前套接口在传输层上有bind的实现，则直接调用传输层接口上的bind(),直接进行bind操作即可，否则进行下面的操作。目前只有SOCK_RAW类型的套接口的传输层接口实现了bind接口，为raw_bind() */
    if (sk->ops->bind)
        return sk->ops->bind(sock->sk, skaddr);
    /* bind函数的src_port==0时，由内核随机分配端口，我们就无需考虑端口地址重复占用的问题 */
    if (sk->sk_addr.src_port) {
        return err;
    }
    /* 绑定本地IP地址 */
    if (!local_address(skaddr->src_addr))
        return err;
    /* 绑定地址 */
    sk->sk_addr.src_addr = skaddr->src_addr;
    /* 绑定端口 */
    if (sk->ops->set_port) {
        err = sk->ops->set_port(sk, skaddr->src_port);
        if (err < 0) {
            sk->sk_addr.src_addr = 0;
            return err;
        }
    } else {
        sk->sk_addr.src_port = skaddr->src_port;
    }
    err = 0;
    /* 初始化目的地址、目的端口 */
    sk->sk_addr.dst_addr = 0;
    sk->sk_addr.dst_port = 0;
    return err;
}

static int inet_listen(struct socket *sock, int backlog) {
    struct sock *sk = sock->sk;
    int err = -1;
    /* 判断套接字类型。SOCK_DGRAM和SOCK_RAW类型不支持listen，只有SOCK_STREAM类型支持listen接口 */
    if (sock->type!=SOCK_STREAM){
        return -1;
    }
    if (sk)
        err=sk->ops->listen(sk,backlog);
    return err;
}

static int inet_accept(struct socket *sock,struct socket *newsock,struct sock_addr *skaddr){
    struct sock *sk = sock->sk;
    struct sock *newsk;
    int err=-1;
    if(!sk)
        return err;
    /* accept()返回一个新的文件描述符，指向一个连接到客户的新套接口文件。而用于listen的套接口仍然是未连接的，并准备接收下一个连接 */
    newsk=sk->ops->accept(sk);
    if(newsk){
        newsock->sk=get_sock(newsk);
        if (skaddr){
            skaddr->src_addr=newsk->sk_addr.dst_addr;
            skaddr->src_port=newsk->sk_addr.dst_port;
        }
        err=0;
    }
    return err;
}

static int inet_connect(struct socket *sock, struct sock_addr *skaddr){
    struct sock *sk = sock->sk;
    int err=-1;
    if (!skaddr->dst_port || !skaddr->dst_addr)
        return err;
    /* 如果现有sock中已设置对端端口号 */
    if(sk->sk_addr.dst_port)
        return err;
    if (!sk->sk_addr.src_port&&sock_autobind(sk)<0)
        return err;
    struct
}

/* inet地址族初始化函数 */
void inet_init(void) {
    raw_init();
    udp_init();
    tcp_init();
}