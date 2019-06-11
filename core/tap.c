//
// Created by Kyrie on 2019/4/29.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../include/ethernet.h"

static skfd;

/**
 * 创建tap设备
 * @param dev
 * @return
 */
int tap_alloc(char *dev){
    struct ifreq ifr;
    int fd,err;

    fd = open(TUN_TAP_DEV,O_RDWR);
    if(fd<0){
        perror("open");
        return -1;
    }

    memset(&ifr,0, sizeof(ifr));
    ifr.ifr_flags = IFF_NO_PI;
    if(!strncmp(dev,"tun",3)){
        ifr.ifr_flags |= IFF_TUN;
    } else if(!strncmp(dev,"tap",3)){
        ifr.ifr_flags |= IFF_TAP;
    } else{
        perror("I don't recongnize device %s as a TUN or TAP device");
    }
    if(strlen(dev)>3){
        strncpy(ifr.ifr_name,dev,IFNAMSIZ);
    }
    if((err=ioctl(fd,TUNSETIFF,(void *)&ifr)) <0){
        perror("ioctl TUNSETIFF");
        close(fd);
        return err;
    }
    return fd;
}

void set_tap(void){
    skfd = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP);
    if (skfd<0)
        perror("socket PF_INET");
}

void unset_tap(void){
    close(skfd);
}

void setflag_tap(unsigned char *name,unsigned short flags,int set){
    struct ifreq ifr={};
    strcpy(ifr.ifr_name,(char *)name);

    if (ioctl(skfd, SIOCGIFFLAGS, (void *) &ifr) < 0) { /* 获取接口标志 */
        close(skfd);
        perror("socket SIOCGIFFLAGS");
    }

    if (set != 0) {
        ifr.ifr_flags |= flags;
    } else {
        ifr.ifr_flags &= ~flags & 0xffff;
    }
    if(ioctl(skfd,SIOCSIFFLAGS,(void *)&ifr) <0 ){ /* 设置接口标志 */
        close(skfd);
        perror("socket SIOCGIFFLAGS");
    }
}

void open_tap(unsigned char *name){
    setflag_tap(name, IFF_UP | IFF_RUNNING, 1);
}

void close_tap(unsigned char *name){
    setflag_tap(name, IFF_UP | IFF_RUNNING, 0);
}

/**
 * tap设备设置持久化模式,即使设备退出后也不销毁
 * @param fd
 * @return
 */
int setpersist_tap(int fd){
    if (ioctl(fd, TUNSETPERSIST, 1) < 0) {
        perror("ioctl TUNSETPERSIST");
        return -1;
    }
    return 0;
}

void getmtu_tap(unsigned char *name,int *mtu){
    struct ifreq ifr ={};
    strcpy(ifr.ifr_name, (char *) name);
    if (ioctl(skfd, SIOCGIFMTU, (void *) &ifr) < 0) {
        close(skfd);
        perror("ioctl SIOCGIFHWADDR");
    }
    *mtu = ifr.ifr_mtu;
}

void getname_tap(int fd,unsigned char *name){
    struct ifreq ifr={};
    if (ioctl(fd, SIOCGIFNAME, (void *) &ifr) < 0) {
        perror("ioctl SIOCGIFHWADDR");
    }
    strcpy((char *) name, ifr.ifr_name);
}

void gethwaddr_tap(int fd, unsigned char *hwa){
    struct ifreq ifr;
    memset(&ifr,0, sizeof(ifr));
    if (ioctl(fd, SIOCGIFHWADDR, (void *) &ifr) < 0) {
        perror("ioctl SIOCGIFHWADDR");
    }

    hwacpy(hwa,ifr.ifr_hwaddr.sa_data);

    printf("mac addr:%02x:%02x:%02x:%02x:%02x:%02x", hwa[0], hwa[1], hwa[2], hwa[3], hwa[4], hwa[5]);
}

void getipaddr_tap(unsigned char *name, unsigned int *ipaddr) {
    struct ifreq ifr={};
    struct sockaddr_in *saddr;

    strcpy(ifr.ifr_name, (char *) name);
    if (ioctl(skfd, SIOCGIFADDR, (void *) &ifr) < 0) {
        close(skfd);
        perror("socket SIOCGIFADDR");
    }
    saddr = (struct sockaddr_in *)&ifr.ifr_addr;
    *ipaddr = saddr->sin_addr.s_addr;
}

void setipaddr_tap(unsigned char *name,unsigned int ipaddr){
    struct ifreq ifr={};
    struct sockaddr_in *saddr;

    strcpy(ifr.ifr_name,(char *)name);
    saddr = (struct sockaddr_in *)&ifr.ifr_addr;
    saddr->sin_family = AF_INET;
    saddr->sin_addr.s_addr = ipaddr;
    if (ioctl(skfd, SIOCSIFADDR, (void *) &ifr) < 0) {
        close(skfd);
        perror("socket SIOCSIFADDR");
    }
}

void setnetmask_tap(unsigned char *name,unsigned int netmask){
    struct ifreq ifr ={};
    struct sockaddr_in *saddr;

    strcpy(ifr.ifr_name, (char *) name);

    saddr = (struct sockaddr_in *)&ifr.ifr_netmask;
    saddr->sin_family = AF_INET;
    saddr->sin_addr.s_addr = netmask;
    if (ioctl(skfd, SIOCSIFNETMASK, (void *) &ifr) < 0) {
        close(skfd);
        perror("socket SIOCSIFNETMASK");
    }
}

void delete_tap(int fd){
    if(ioctl(fd,TUNSETPERSIST,0)<0)
        return;

    close(fd);
}

