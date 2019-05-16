//
// Created by JKerving on 2019/5/2.
// TAP设备驱动程序

#include "../include/netif.h"
#include "../include/list.h"
#include "../include/tap.h"
#include "../include/lib.h"
#include "../include/netconfig.h"
#include "../include/ethernet.h"
#include <unistd.h>


struct tapdev *tap;
struct net_device *veth;

static int tap_dev_init(){
    tap = xmalloc(sizeof(*tap));
    tap->fd = tap_alloc("tap0");
    if (tap->fd < 0) {
        free(tap);
        return -1;
    }
//khkjh8yii


    if (setpersist_tap(tap->fd) < 0) {
        close(tap->fd);
    }
    set_tap(); /* 设置tap设备接口信息 */
    getname_tap(tap->fd,tap->dev.net_name);
    getmtu_tap(tap->dev.net_name,&tap->dev.net_mtu);

    /* ifndef CONFIG_TOP1 */
    gethwaddr_tap(tap->fd,tap->dev.net_hwaddr);
    setipaddr_tap(tap->dev.net_name,FAKE_TAP_ADDR);
    getipaddr_tap(tap->dev.net_name,&tap->dev.net_ipaddr);
    setnetmask_tap(tap->dev.net_name,FAKE_TAP_NETMASK);
    open_tap(tap->dev.net_name);

    unset_tap();
    list_init(&tap->dev.net_list);
}

static int veth_dev_init(struct net_device *dev){
    if(tap_dev_init()<0) {
        perror("Cannot init tapdevice");
    }

    /* 初始化veth，用于提供给协议栈设备信息 */
    dev->net_mtu = tap->dev.net_mtu;
    dev->net_ipaddr = FAKE_IPADDR;
    dev->net_mask = FAKE_NETMASK;
    hwacpy(dev->net_hwaddr,FAKE_HWADDR);
    return 0;
}

static void veth_dev_exit(struct net_device *dev){
    if(dev!=veth){
        perror("Net Device Error");
    }
    delete_tap(tap->fd);
}

struct net_device_stats *get_netdev_stats(struct net_device *dev){
    return dev->netdev_stats;
}

unsigned int localnet(struct net_device *dev){
    return (dev)->net_ipaddr & (dev)->net_mask;
}

static int veth_xmit(struct net_device *dev, struct pk_buff *pkb){
    int l;
    l = write(tap->fd, pkb->pk_data, pkb->pk_len);
    if(l!=pkb->pk_len) {
        dbg("write net_device");
        dev->netdev_stats.tx_errors++;
    } else{
        dev->netdev_stats.tx_packets++;
        dev->netdev_stats.tx_bytes+=l;
        dbg("write net_device size:%d\n",l);
    }
    return l;
}

static int veth_recv(struct pk_buff *pkb){
    int l;
    l = read(tap->fd,pkb->pk_data,pkb->pk_len);
    if (l <= 0) {
        dbg("read net device");
        veth->netdev_stats.rx_errors++;
    } else{
        dbg("read net device size:%d\n",l);
        veth->netdev_stats.rx_packets++;
        veth->netdev_stats.rx_bytes += l;
        pkb->pk_len = l;
    }
    return l;
}

/**
 * 将接收到的pkb传递给协议栈上层
 */
static void veth_rx(void){
    struct pk_buff *pkb = alloc_netdev_pkb(veth);
    /* veth设备接收数据报文pkb */
    if (veth_recv(pkb) > 0) {
        ethernet_in(veth,pkb);/* 在ethernet_in方法中判断数据报文协议类型 */
    } else{
        free_pkb(pkb);/* 如果recv失败，直接丢弃pkb */
    }
}

void veth_poll(void){
    struct pollfd pfd = {};
    int ret_val;
    /* 阻塞程序，等待读取事件发生 */
    while (1){
        pfd.fd = tap->fd;
        pfd.events=POLLIN;
        pfd.revents = 0;

        ret_val = poll(&pfd,1,-1);
        if(ret_val<=0) {
            perror("poll /dev/net/tun");
        }
        /*获取packet，调用对应的handler处理数据报文*/
        veth_rx();
    }
}

static struct netdev_ops veth_ops ={
        .init = veth_dev_init,
        .hard_xmit = veth_xmit,
        .get_netdev_stats = get_netdev_stats,
        .localnet = localnet,
        .exit = veth_dev_exit,
};