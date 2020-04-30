#include <zconf.h>
#include <pthread.h>
#include "../../include/list.h"
#include "../../include/sock.h"
#include "../../include/socket.h"
#include "../../include/ip.h"
#include "../../include/udp.h"


#define UDP_HTABLE_SIZE 128
#define UDP_BEST_UPDATE 10 /* 没有指定绑定端口时，会自动选择一个合适的端口进行绑定。这里的常量表示每自动绑定10次，则更新一次best端口号 */
#define UDP_PORT_MIN    0x8000
#define UDP_PORT_MAX    0xf000

#define udp_hash_slot(hash) (&udp_table.slot[hash])
#define udp_slot(port)  udp_hash_slot(port & (UDP_HTABLE_SIZE-1))
#define udp_best_slot() udp_hash_slot(udp_table.best_slot)
#define udp_hash_head(hash) (&udp_hash_slot(hash)->head)
#define udp_slot_head(port) (&udp_slot(port)->head)
#define udp_best_slot_head()    (&udp_best_slot()->head)

//
// Created by 张家望 on 2020/4/28.
//
struct hash_slot {
    struct hlist_head head;
    int used;
};

struct hash_table {
    struct hash_slot slot[UDP_HTABLE_SIZE];
    int best_slot;
    int best_update;
    pthread_mutex_t mutex;
};

static struct hash_table udp_table;

static int udp_get_port_slow();

static unsigned short udp_id;
/*本地端口区间范围*/
int sysctl_local_port_range[2] = {UDP_PORT_MIN, UDP_PORT_MAX};

static void udp_htable_lock(void) {
    pthread_mutex_lock(&udp_table.mutex);
}

static void udp_htable_unlock(void) {
    pthread_mutex_unlock(&udp_table.mutex);
}

static int udp_hash(struct sock *sk) {
    sock_add_hash(sk, udp_hash_head(sk->hash));
    return 0;
}


static int __port_used(unsigned short port, struct hlist_head *head) {
    struct hlist_node *node;
    struct sock *sk;
    //遍历udp_htable中best_slot槽位的链表，查找端口是否被占用
    hlist_for_each_sock(sk, node, head)if (sk->sk_addr.src_port == _htons(port))
            return 1;
    return 0;
}

static int port_used(unsigned short port) {
    return __port_used(port, udp_slot_head(port));
}

/**
 *
 * @param sk
 * @param nport n表示网络字节序
 * @return
 */
static int udp_set_port(struct sock *sk, unsigned short nport) {
    udp_htable_lock();

    if ((nport && port_used(_ntohs(nport))) || (!nport && (nport =)))
}

static inline unsigned short udp_get_best_port(void) {
    unsigned short port = udp_table.best_slot + UDP_PORT_MIN;
    struct hash_slot *best = udp_best_slot();

    /* 找到未被占用的端口号 */
    if (best->used != 0) {
        while (port < UDP_PORT_MAX) {
            if (__port_used(port, &best->head))
                break;
            port += UDP_HTABLE_SIZE;
        }
        if (port >= UDP_PORT_MAX)
            return 0;
    }
    best->used++;
    return port;
}

static int udp_get_port_slow(void) {
    int best_slot = udp_table.best_slot;
    int best_slot_used = udp_best_slot()->used;
    int i;
    /*递增遍历散列表找到used<best_slot.used的索引值，只要桶内控制块数量最小的*/
    for (i = 0; i < UDP_HTABLE_SIZE; i++) {
        if (udp_hash_slot(i)->used < best_slot_used) {
            best_slot_used = udp_hash_slot(i)->used;
            best_slot = i;
        }
    }
    /*没有找到合适的端口号，说明htable已经满了*/
    if (best_slot == udp_table.best_slot)
        return 0;

    udp_table.best_slot = best_slot;
    udp_table.best_update = UDP_BEST_UPDATE;/*重置UPDATE次数*/
    return udp_get_best_port();
}

/**
 *
 * @return 返回值为网络序端口号
 */
static unsigned short udp_get_port(void) {
    struct hash_slot *best = udp_best_slot();
    unsigned short port;

    port = udp_get_best_port();
    if (port == 0) {
        return udp_get_port_slow();
    }

    /*更新htable的best slot*/
    if (--udp_table.best_update <= 0) {
        int i;
        udp_table.best_update = UDP_BEST_UPDATE;
        for (i = 0; i < UDP_HTABLE_SIZE; i++) {
            if (udp_table.slot[i].used < best->used) {
                udp_table.best_slot = i;
                break;
            }
        }
    }
    return _htons(port);
}

/*更新best_slot*/
static inline void udp_update_best(int hash) {
    if (udp_table.best_slot != hash) {
        if (udp_hash_slot(hash)->used < udp_best_slot()->used) {
            udp_table.best_slot = hash;
        }
    }
}

static int udp_set_sport(struct sock *sk, unsigned short nport) {
    int err = -1;
    udp_htable_lock();

    if ((nport && port_used(_ntohs(nport))) || (!nport && !(nport = udp_get_port()))) {
        udp_htable_unlock();
        return err;
    }

    int hash_bucket_index = _ntohs(nport) & (UDP_HTABLE_SIZE - 1);
    udp_update_best(hash_bucket_index);
    sk->hash = hash_bucket_index;
    sk->sk_addr.src_port = nport;
    if (sk->ops->hash)
        sk->ops->hash(sk);
    udp_htable_unlock();
    return 0;
}

/*解绑端口号*/
static void udp_unset_sport(struct sock *sk) {
    struct hash_slot *slot = udp_hash_slot(sk->hash);
    slot->used--;
    /*解绑之后，由于slot->used引用数减少，所以这里重新update_best_slot*/
    udp_update_best(sk->hash);
}

static void udp_unhash(struct sock *sk) {
    udp_unset_sport(sk);
    sock_del_hash(sk);
}

/*构造udp数据报*/
static int udp_init_pkb(struct sock *sk, struct pk_buff *pkb, void *buf, int size, struct sock_addr *skaddr) {
    struct ip *iphdr = pkb2ip(pkb);/*拆包*/
    struct udp *udphdr = (struct udp *) iphdr->data;
    /*填充ip包头部结构*/
    iphdr->ihl = IP_HRD_SZ / 4;
    iphdr->version = IP_VERSION_4;
    iphdr->tos = 0;
    iphdr->tot_len = _htons(pkb->pk_len - ETH_HRD_SZ);
    iphdr->id = _htons(udp_id);
    iphdr->frag_off=0;
    iphdr->ttl=UDP_DEFAULT_TTL;
}

void udp_init(void) {
    struct hash_slot *slot;
    int i;
    /* 初始化udp_htable */
    for (slot = udp_hash_slot(i); i < UDP_HTABLE_SIZE; i++, slot++) {
        hlist_head_init(&slot->head);
        slot->used = 0;
    }
    udp_table.best_slot = 0;
    udp_table.best_update = UDP_BEST_UPDATE;
    pthread_mutex_init(&udp_table.mutex, NULL);
    udp_id = 0;
}


