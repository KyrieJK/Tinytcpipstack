//#include <endian.h>
#include <stdio.h>

//
// Created by JKerving on 2019/4/21.
//

#define PROTO_TCP 6
#define PROTO_UDP 17

#define HOST_LITTLE_ENDIAN

#ifdef HOST_LITTLE_ENDIAN

#define _ntohs(net) _htons(net)

#define _ntohl(net) _htonl(net)

static inline unsigned short _htons(unsigned short host){
    return (host >> 8) | ((host << 8) & 0xff00);
}

static inline unsigned int _htonl(unsigned int host){
    return ((host & 0x000000ff) << 24) | ((host & 0x0000ff00) << 8) | ((host & 0x00ff0000) >> 8) |
           ((host & 0xff000000) >> 24);
}

#endif
static inline unsigned int sum(unsigned short *data, int size, unsigned int cksum){
    //对数据包头部中的每16bit进行二进制求和
    //参数data是指向unsigned short的指针，unsigned short为2字节，即16bit
    //刚开始指向的是数据包首部的起始地址，size为首部的大小
    //while循环是将数据包首部的内容以16bit为单元加在一起，如果没有整除(即size还有余下不足16bit的部分)
    //则加上余下的部分。此时，cksum是相加后的结果，这个结果往往超出了16bit，因为校验和是16bit的
    //所以checksum函数是将高16bit和计算得到的cksum再加工
    while (size>1){
        cksum+=*data++;
        size-= sizeof(data);
    }
    if(size){
        cksum += _ntohs(((*(unsigned char *) data) & 0xff) << 8);
    }
    return cksum;
}

static inline unsigned short checksum(unsigned short *data,int size,unsigned int cksum){
    cksum = sum(data, size, cksum);
    //将cksum的低16位与高16位相加
    cksum = (cksum & 0xffff) + (cksum >> 16);
    //将进位到高位的16bit与低16位再相加。因为第一步相加时很可能会再产生进位，因此要再次把进位移到低16bit进行相加
    cksum = (cksum & 0xffff) + (cksum >> 16);
    //将该16bit的值取反，存入校验和字段
    return (~cksum & 0xffff);
}

/**
 * ip首部校验和函数
 * @param data
 * @param size
 * @return
 */
unsigned short ip_chksum(unsigned short *data,int size){
    return checksum(data,size,0);
}

/**
 * icmp首部校验和函数
 * @param data
 * @param size
 * @return
 */
unsigned short icmp_chksum(unsigned short *data,int size){
    return checksum(data,size,0);
}

static inline unsigned short tcp_udp_chksum(unsigned int src,unsigned int dst,unsigned short proto,unsigned short len,unsigned short *data){
    unsigned int sum;
    /*计算tcp伪首部校验和*/
    sum = _htons(proto) + _htons(len);
    sum += src;
    sum += dst;
    /*计算tcp首部和数据部分的校验和*/
    return checksum(data,len,sum);
}

unsigned short tcp_chksum(unsigned int src,unsigned int dst,unsigned short len,unsigned short *data){
    return tcp_udp_chksum(src,dst,PROTO_TCP,len,data);
}

unsigned short udp_chksum(unsigned int src,unsigned int dst,unsigned short len,unsigned short *data){
    return tcp_udp_chksum(src,dst,PROTO_UDP,len,data);
}

