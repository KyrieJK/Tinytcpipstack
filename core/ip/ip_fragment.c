//
// Created by 张家望 on 2020/5/6.
//

#include "../../include/netif.h"
#include "../../include/ethernet.h"
#include "../../include/lib.h"
#include "../../include/netconfig.h"
#include "../../include/list.h"
#include "../../include/ip.h"
#include "../../include/checksum.h"

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
        pkb=list_first_entry(&(frag)->frag_pkb,struct pk_buff,pk_list);
        list_del(&pkb->pk_list);
        free_pkb(pkb);
    }
    free(frag);
}

int insert_frag(struct pk_buff *pkb,struct fragment *frag){

}

struct fragment *lookup_frag(struct ip *iphdr){
    struct fragment *frag;
    list_for_each_entry(frag,&frag_head,frag_list)
        if (frag->frag_id==iphdr->id&&frag->frag_pro==iphdr->protocol&&frag->frag_src==iphdr->saddr&&frag->frag_dst==iphdr->daddr)
            return frag;
    return NULL;
}

/**
 * 根据源IP包构造分片
 * @param pkb
 * @param orig
 * @param hlen
 * @param dlen
 * @param off
 * @param mf_bit
 * @return
 */
struct pk_buff *ip_frag(struct pk_buff *pkb,struct ip *orig,int hlen,int dlen,int off,unsigned short mf_bit){
    struct pk_buff *fragpkb;
    struct ip *fraghdr;

    /*分配一个新的pkb buffer
     * 数据长度+IP头部长度+链路长度
     * */
    fragpkb=alloc_pkb(ETH_HRD_SZ+hlen+dlen);
    fragpkb->pk_protocol=pkb->pk_protocol;
    fragpkb->pk_type=pkb->pk_type;
    fragpkb->pk_indev=pkb->pk_indev;
    fragpkb->pk_rtdst=pkb->pk_rtdst;

    fraghdr=pkb2ip(fragpkb);
    /*
     * 复制数据部分和IP头
     * */
    memcpy(fraghdr,orig,hlen);
    memcpy((void *)fraghdr+hlen,(void *)orig+hlen+off,dlen);
    /*
     * 更新长度字段
     * */
    fraghdr->tot_len=_htons(hlen+dlen);

    mf_bit|=(off>>3);
    fraghdr->frag_off=_htons(mf_bit);
    fraghdr->checksum=0;
    fraghdr->checksum=ip_chksum((unsigned short *)fraghdr,iphlen(fraghdr));
    return fragpkb;
}



