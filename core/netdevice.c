#include "../include/netif.h"
#include "../include/ethernet.h"
#include "../include/lib.h"
#include "../include/list.h"
#include "../include/netconfig.h"

/* 本地设备抽象结构存储链表 */
struct list_head net_devices;

extern void loop_init(void);
extern void veth_init(void);
extern void loop_exit(void);
extern void veth_exit(void);
extern void veth_poll(void);

/*  分配net_device结构实例的内存
    将net_device加入到netdevice list中
    配置初始化net_device结构实例的成员
 */
struct net_device *netdev_alloc(char *devstr,struct netdev_ops *netops){
    struct net_device *dev;
    dev = xcalloc(sizeof(*dev));
    list_add_tail(&dev->net_list,&net_devices);
    dev->net_name[IFNAMESIZE-1]='\0';
    strncpy((char *)dev->net_name,devstr,IFNAMESIZE-1);
    dev->net_ops = netops;
    if (netops&&netops->init)
    {
        netops->init(dev);
    }
    return dev;
}

void netdev_free(struct net_device *dev){
    if (dev->net_ops&&dev->net_ops->exit)
    {
        dev->net_ops->exit(dev);
    }
    list_del(&dev->net_list);
    free(dev);
}

/**
 * 中断事件
 * 当有pollin事件到来时，会发生触发中断
 * **/
void netdev_interrupt(void){
    veth_poll();
}

void netdev_init(void){
    list_init(&net_devices);
    loop_init();
    veth_init();
}

void netdev_exit(void){
    veth_exit();
    loop_exit();
}

void netdev_tx(struct net_device *dev,struct pk_buff *pkb,int len,unsigned short protocol,unsigned char *dst){
    struct ethernet_hdr *ethdr = (struct ethernet_hdr *)pkb->pk_data;

    ethdr->protocol = _htons(protocol);
    hwacpy(ethdr->eth_dst,dst);
    hwacpy(ethdr->eth_src,dev->net_hwaddr);
    pkb->pk_len = len+ETH_HRD_SZ;

    dev->net_ops->hard_xmit(dev,pkb);
    free_pkb(pkb);
}

/* 根据IP地址筛选网络设备 */
int local_address(unsigned int addr){
    struct net_device *dev;
    if(!addr){
        return 1;
    }
    /* loopback 设备IP地址 */
    if(localnet(loop) == (loop->net_mask & addr)){
        return 1;
    }
    /* 遍历设备链表，分别对比IP地址 */
    list_for_each_enrty(dev,&net_devices,net_list){
        if(dev->net_ipaddr == addr)
            return 1;
    }
    return 0;
}




