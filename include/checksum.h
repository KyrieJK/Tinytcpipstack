//
// Created by 张家望 on 2020/5/1.
//

#ifndef TINYTCPIPSTACK_CHECKSUM_H
#define TINYTCPIPSTACK_CHECKSUM_H

extern unsigned short ip_chksum(unsigned short *,int);
extern unsigned short tcp_chksum(unsigned int,unsigned int,unsigned short,unsigned short *);
extern unsigned short udp_chksum(unsigned int,unsigned int,unsigned short,unsigned short *);
extern unsigned short icmp_chksum(unsigned short *,int);

#endif //TINYTCPIPSTACK_CHECKSUM_H
