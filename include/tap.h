//
// Created by JKerving on 2019/4/29.
//

#ifndef TINYTCPIPSTACK_TAP_H
#define TINYTCPIPSTACK_TAP_H

#define TUNTAPDEV "/dev/net/tun"

extern int tap_alloc(char *dev);

extern void set_tap(void);

extern void unset_tap(void);

extern void setflag_tap(unsigned char *name,unsigned short flags,int set);

extern void open_tap(unsigned char *name);

extern void close_tap(unsigned char *name);

extern int setpersist_tap(int fd);

extern void getmtu_tap(unsigned char *name,int *mtu);

extern void getname_tap(int fd,unsigned char *name);

extern void gethwaddr_tap(int fd, unsigned char *hwa);

extern void getipaddr_tap(unsigned char *name, unsigned int *ipaddr);

extern void setipaddr_tap(unsigned char *name,unsigned int ipaddr);

extern void setnetmask_tap(unsigned char *name,unsigned int netmask);

extern void delete_tap(int fd);

#endif //TINYTCPIPSTACK_TAP_H
