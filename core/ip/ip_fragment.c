//
// Created by 张家望 on 2020/5/6.
//

#include "../../include/netif.h"
#include "../../include/ethernet.h"
#include "../../include/lib.h"
#include "../../include/netconfig.h"
#include "../../include/list.h"
#include "../../include/ip.h"

static LIST_HEAD(frag_head); /*数据包头*/

static inline int complete_frag(struct fragment *frag) {
    return frag->frag_flags & FRAG_COMPLETE;
}

static inline int full_frag(struct fragment *frag) {
    return (((frag->frag_flags & FRAG_FL_IN) == FRAG_FL_IN) && (frag->frag_rsize == frag->frag_size));
}

struct fragment *new_frag(struct ip *iphdr){
    struct fragment *frag;
    frag=xmalloc(sizeof(*frag));
    frag->frag_ttl=FRAG_TIME;
    frag->frag_id=iphdr->id;
    frag->frag_src=iphdr->saddr;
    frag->frag_dst=iphdr->daddr;
    frag->frag_pro=iphdr->protocol;
    frag->frag_hlen=0;
    frag->frag_size=0;
    frag->frag_rsize=0;
    frag->frag_flags=0;
    list_add(&frag->frag_list,&frag_head);
    list_init(&frag->frag_pkb);
    return frag;
}

void delete_frag(struct fragment *frag){
    struct pk_buff *pkb;
    list_del(&frag->frag_list);
    while (!list_empty(&frag->frag_pkb)){
        pkb=
    }
}


