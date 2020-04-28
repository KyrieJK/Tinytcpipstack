#include <zconf.h>
#include <pthread.h>
#include "../../include/list.h"
#include "../../include/sock.h"
#include "../../include/socket.h"

#define UDP_HTABLE_SIZE 128

#define udp_hash_slot(hash) (&udp_table.slot[hash])
#define udp_slot(port)  udp_hash_slot(port & (UDP_HTABLE_SIZE-1))
#define udp_hash_head(hash) (&udp_hash_slot(hash)->head)
#define udp_slot_head(port) (&udp_slot(port)->head)

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
static unsigned short udp_id;

static void udp_htable_lock(void) {
    pthread_mutex_lock(&udp_table.mutex);
}

static void udp_htable_unlock(void) {
    pthread_mutex_unlock(&udp_table.mutex);
}

static int udp_hash(struct sock *sk) {
    sock_add_hash(sk, udp_hash_head(sk->hash))
}

static int __port_used(unsigned short port, struct hlist_head *head) {
    struct hlist_node *node;
    struct sock *sk;
    //遍历udp_htable查找端口是否被占用
    hlist_for_each_sock(sk, node, head)
        if (sk->sk_addr.src_port == _htons(port))
            return 1;
    return 0;
}

static int port_used(unsigned short port){
    return __port_used(port,udp_slot_head(port));
}

static unsigned short udp_get_port(void){

}


