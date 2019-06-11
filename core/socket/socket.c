/**
 * 实现套接口层的调用
 */

#include "lib.h"
#include "socket.h"

static struct socket *alloc_socket(int family,int type){
    struct socket *sock;
    sock = xcalloc(sizeof(*sock));
    sock->state = SS_UNCONNECTED;
    sock->family = family;
    sock->type = type;
    
}