//
// Created by JKerving on 2019/5/3.
//

#include <ctype.h>
#include <stdio.h>

#include "../include/netif.h"
#include "../include/ethernet.h"
#include "../include/lib.h"

#define MAX_PKBS 300

int free_pkbs = 0;
int alloc_pkbs = 0;

/**
 * 查询pkb数量
 */
void pkb_amount(){
    if((alloc_pkbs-free_pkbs)>MAX_PKBS) {
        dbg("exceed MAX_PKBS");
        exit(EXIT_FAILURE);
    }
}

/**
 * 用来从pkb尾部删除数据
 * @param pkb
 * @param len
 */
void pkb_trim(struct pk_buff *pkb,int len){
    if (pkb->pk_len > len) {
        pkb->pk_len=len;
        if (realloc(pkb, sizeof(*pkb) + len) == NULL) {
            perror("realloc");
        }
    }
}

/**
 * 分配pkb结构
 * @param size
 * @return
 */
struct pk_buff *alloc_pkb(int size){
    struct pk_buff *pkb;
    pkb = xcalloc(sizeof(*pkb)+size);
    pkb->pk_len = size;
    pkb->pk_protocol = 0xffff;
    pkb->pk_type = 0;
    pkb->pk_refcnt = 1;
    pkb->pk_indev = NULL;
    pkb->pk_rtdst = NULL;
    list_init(&pkb->pk_list);
    alloc_pkbs++;
    pkb_amount();
    return pkb;
}

/**
 * pkb是整个协议栈中传输网络数据报文的载体
 * pkb用来存储数据报文的内容和各层协议头
 * alloc_netdev_pkb首先分配的是数据链路层的pkb结构体
 * @param dev
 * @return
 */
struct pk_buff *alloc_netdev_pkb(struct net_device *dev){
    return alloc_pkb(dev->net_mtu+ETH_HRD_SZ);
}

struct pk_buff *copy_pkb(struct pk_buff *pkb){
    struct pk_buff *clonepkb;
    clonepkb = xmalloc(sizeof(*clonepkb)+pkb->pk_len);
    memcpy(clonepkb,pkb, sizeof(*clonepkb)+pkb->pk_len);
    clonepkb->pk_refcnt=1;
    list_init(&clonepkb->pk_list);
    alloc_pkbs++;
    pkb_amount();
    return clonepkb;
}

void free_pkb(struct pk_buff *pkb){
    if(--pkb->pk_refcnt<=0) {
        free_pkbs++;
        free(pkb);
    }
}

struct pk_buff get_pkb(struct pk_buff *pkb){
    pkb->pk_refcnt++;
    return *pkb;
}

