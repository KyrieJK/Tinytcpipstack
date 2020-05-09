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

/**
 * 判断分片是否全部到达
 * @param frag
 * @return
 */
static inline int complete_frag(struct fragment *frag) {
    return frag->frag_flags & FRAG_COMPLETE;
}

static inline int full_frag(struct fragment *frag) {
    return (((frag->frag_flags & FRAG_FL_IN) == FRAG_FL_IN) && (frag->frag_rsize == frag->frag_size));
}

struct fragment *new_frag(struct ip *iphdr) {
    struct fragment *frag;
    frag = xmalloc(sizeof(*frag));
    frag->frag_ttl = FRAG_TIME;
    frag->frag_id = iphdr->id;
    frag->frag_src = iphdr->saddr;
    frag->frag_dst = iphdr->daddr;
    frag->frag_pro = iphdr->protocol;
    frag->frag_hlen = 0;
    frag->frag_size = 0;
    frag->frag_rsize = 0;
    frag->frag_flags = 0;
    list_add(&frag->frag_list, &frag_head);
    list_init(&frag->frag_pkb);
    return frag;
}

void delete_frag(struct fragment *frag) {
    struct pk_buff *pkb;
    list_del(&frag->frag_list);
    while (!list_empty(&frag->frag_pkb)) {
        pkb = list_first_entry(&(frag)->frag_pkb, struct pk_buff, pk_list);
        list_del(&pkb->pk_list);
        free_pkb(pkb);
    }
    free(frag);
}

/**
 * 通过偏移量计算偏移字节数。片偏移就是某片在原分组的相对位置，以8个字节为偏移单位。
 * @param iphdr
 * @return
 */
int get_frag_off(struct ip *iphdr) {
    return ((((iphdr)->frag_off) & IP_FRAG_OFFSET) * 8);
}

/**
 * 重组Fragment
 * @param frag
 * @return
 */
struct pk_buff *reass_frag(struct fragment *frag){
    struct pk_buff *pkb,*fragpkb;
    struct ip *fraghdr;
    int hlen,len;

}

/**
 * 插入分片相应的位置，重组时保证分片的有序
 * @param pkb
 * @param frag
 * @return
 */
int insert_frag(struct pk_buff *pkb, struct fragment *frag) {
    struct pk_buff *fragpkb;
    struct ip *iphdr, *fraghdr;
    struct list_head *pos;
    int frag_offset, hlen;

    if (complete_frag(frag)) {
        ferr("redundant fragment for the completed reassembled packet");
        free_pkb(pkb);
    }

    iphdr = pkb2ip(pkb);
    frag_offset = get_frag_off(iphdr);
    hlen = iphlen(iphdr);

    /*判断是否为最后一个分片*/
    if ((iphdr->frag_off & IP_FRAG_MF) == 0) {
        if (frag->frag_flags & FRAG_LAST_IN) {
            ferr("重复的最终分片");
            free_pkb(pkb);
        }
        frag->frag_flags |= FRAG_LAST_IN;/*设置frag标志位为FRAG_LAST_IN*/
        frag->frag_size = frag_offset + iphdr->tot_len - hlen;/*接收到的分片总大小*/
        pos = frag->frag_pkb.prev;
        list_add(&pkb->pk_list, pos);
        frag->frag_rsize += iphdr->tot_len - hlen;/*已接收到的分片大小*/
        if (full_frag(frag))
            frag->frag_flags |= FRAG_COMPLETE;
        return 0;
    }

    /*普通Fragment的插入方法*/
    pos = &frag->frag_pkb;
    /**
     * 找出第一个比当前分片偏移小的节点
     */
    list_for_each_entry_reverse(fragpkb, &frag->frag_pkb, pk_list) {
        fraghdr = pkb2ip(fragpkb);
        if (frag_offset == get_frag_off(fraghdr)) {
            ferr("重复的分片结构");
            free_pkb(fragpkb);
        }
        if (frag_offset > get_frag_off(fraghdr)) {
            pos = &fragpkb->pk_list;
            /*分片头部check*/
            if (frag->frag_hlen && frag->frag_hlen != hlen) {
                ferr("eror ip fragment");
                free_pkb(pkb);
            } else {
                frag->frag_hlen = hlen;
            }

            /*再次check 找到的fraghdr offset 与当前分片的offset大小对比*/
            if (fraghdr && get_frag_off(fraghdr) + fraghdr->tot_len - hlen > frag_offset) {
                ferr("错误的分片");
                free_pkb(pkb);
            }

            /*判断是否为第一个分片（偏移量offset为0）*/
            if (frag_offset==0)
                frag->frag_flags|=FRAG_FIRST_IN;

            /*插入当前分片到pos分片后面*/
            list_add(&pkb->pk_list,pos);
            frag->frag_rsize+=iphdr->tot_len-hlen;
            if (full_frag(frag))
                frag->frag_flags|=FRAG_COMPLETE;
            return 0;
        }
    }
    /*如果没有符合条件的，就说明没有找到。当前分片就是最小offset的分片*/
    fraghdr = NULL;
    /*当前分片头部check*/
    if (frag->frag_hlen && frag->frag_hlen != hlen) {
        ferr("eror ip fragment");
        free_pkb(pkb);
    } else {
        frag->frag_hlen = hlen;
    }

    /*再次check 找到的fraghdr offset 与当前分片的offset大小对比，在这里fraghdr为NULL*/
    if (fraghdr && get_frag_off(fraghdr) + fraghdr->tot_len - hlen > frag_offset) {
        ferr("错误的分片");
        free_pkb(pkb);
    }

    /*判断是否为第一个分片（偏移量offset为0）*/
    if (frag_offset==0)
        frag->frag_flags|=FRAG_FIRST_IN;

    /*插入当前分片到pos分片后面*/
    list_add(&pkb->pk_list,pos);
    frag->frag_rsize+=iphdr->tot_len-hlen;
    if (full_frag(frag))
        frag->frag_flags|=FRAG_COMPLETE;
    return 0;
}

/**
 * 根据IP头找到分片队列的头指针
 * @param iphdr
 * @return
 */
struct fragment *lookup_frag(struct ip *iphdr) {
    struct fragment *frag;
    list_for_each_entry(frag, &frag_head, frag_list)
        if (frag->frag_id == iphdr->id &&
            frag->frag_pro == iphdr->protocol &&
            frag->frag_src == iphdr->saddr &&
            frag->frag_dst == iphdr->daddr)
            return frag;
    return NULL;
}

/**
 * 根据源IP包构造分片结构体
 * @param pkb
 * @param orig
 * @param hlen
 * @param dlen
 * @param off
 * @param mf_bit
 * @return
 */
struct pk_buff *ip_frag(struct pk_buff *pkb, struct ip *orig, int hlen, int dlen, int off, unsigned short mf_bit) {
    struct pk_buff *fragpkb;
    struct ip *fraghdr;

    /*分配一个新的pkb buffer
     * 数据长度+IP头部长度+链路长度
     * */
    fragpkb = alloc_pkb(ETH_HRD_SZ + hlen + dlen);
    fragpkb->pk_protocol = pkb->pk_protocol;
    fragpkb->pk_type = pkb->pk_type;
    fragpkb->pk_indev = pkb->pk_indev;
    fragpkb->pk_rtdst = pkb->pk_rtdst;

    fraghdr = pkb2ip(fragpkb);
    /*
     * 复制数据部分和IP头
     * */
    memcpy(fraghdr, orig, hlen);
    memcpy((void *) fraghdr + hlen, (void *) orig + hlen + off, dlen);
    /*
     * 更新长度字段
     * */
    fraghdr->tot_len = _htons(hlen + dlen);

    mf_bit |= (off >> 3);
    fraghdr->frag_off = _htons(mf_bit);
    fraghdr->checksum = 0;
    fraghdr->checksum = ip_chksum((unsigned short *) fraghdr, iphlen(fraghdr));
    return fragpkb;
}



