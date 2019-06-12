/**
 * 实现套接口层的系统调用
 */

#include "lib.h"
#include "socket.h"
#include "wait_simulation.h"


static struct socket *alloc_socket(int family,int type){
    struct socket *sock;
    sock = xcalloc(sizeof(*sock));
    sock->state = SS_UNCONNECTED;
    sock->family = family;
    sock->type = type;
    wait_init(&sock->sleep);
    sock->refcnt = 1;
    return sock;
}

static struct socket* get_socket(struct socket *sock){
    sock->refcnt++;
    return sock;
}

static void free_socket(struct socket *sock){
    if(--sock->refcnt<=0){
        __free_socket(sock);
    }
}

static void __free_socket(struct socket *sock){
    if(sock->ops){
        sock->ops->close(sock);
        sock->ops =NULL;
    }

    free(sock);//释放占用内存空间
}

struct socket *_socket(int family,int type,int protocol){
    struct socket *sock = NULL;
    /* 判断是否为INET协议族(本协议栈仅支持INET协议族) */
    if(family != AF_INET){
        return sock;
    }

    sock = alloc_socket(family,type);
    if(sock == NULL){
        return NULL;
    }
    sock->ops = 
}