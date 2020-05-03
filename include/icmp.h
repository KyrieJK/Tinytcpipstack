//
// Created by 张家望 on 2020/5/3.
//

#ifndef TINYTCPIPSTACK_ICMP_H
#define TINYTCPIPSTACK_ICMP_H

/*ICMP报头*/
struct icmphdr{
    unsigned char icmp_type;
    unsigned char icmp_code;
    unsigned short icmp_checksum;
};

#endif //TINYTCPIPSTACK_ICMP_H
