//
// Created by JKerving on 2019/4/22.
//

#ifndef TINYTCPIPSTACK_RING_BUFFER_H
#define TINYTCPIPSTACK_RING_BUFFER_H

/*
 * ring buffer:
 *   .<------------size-------------->.
 *   |                                |
 *   |<-x->|<-RBUFUSED->|<-----y----->|
 *   |     |            |<-RBUFRIGHT->|
 *   +-----++++++++++++++-------------+
 *         |            |
 *         RBUFTAIL     RBUFHEAD
 *   (RBUFFREE = x+y)
 */

struct ring_buf{
    int head;//写指针
    int tail;//读指针
    int size;//buffer大小
    char buf[0];//长度为0的数组,可变数组
};

//写指针位置索引
#define RBUFHEAD(ring_buf) ((ring_buf)->head % (ring_buf->size))
//读指针位置索引
#define RBUFTAIL(ring_buf) ((ring_buf)->tail % (ring_buf->size))
//已写入数据的部分
#define RBUFUSED(ring_buf) ((ring_buf)->head - (ring_buf)->tail)
//未写入数据的部分
#define RBUFUNUSED(ring_buf) ((ring_buf)->size - RBUFUSED(ring_buf))
//写指针具体指向的数据的内存地址
#define RBUFHEAD_ADDR(ring_buf) &(ring_buf)->buf[RBUFHEAD(ring_buf)]
//读指针具体指向的数据的内存地址
#define RBUFTAIL_ADDR(ring_buf) &(ring_buf)->buf[RBUFTAIL(ring_buf)]
//写指针右侧剩余可写入的空间
#define RBUFHEADRIGHT(ring_buf)\
    ((RBUFHEAD_ADDR(ring_buf)>=RBUFTAIL_ADDR(ring_buf)) ?\
        ((ring_buf->size - RBUFHEAD(ring_buf))) :\
            (RBUFTAIL(ring_buf) - RBUFHEAD(ring_buf)))
//读指针右侧剩余可读取的空间
#define RBUFTAILRIGHT(ring_buf)\
    ((RBUFHEAD_ADDR(ring_buf) > RBUFTAIL_ADDR(ring_buf)) ?\
        (RBUFUSED(ring_buf)) :\
            ((ring_buf)->size - RBUFTAIL(ring_buf)))

#endif //TINYTCPIPSTACK_RING_BUFFER_H
