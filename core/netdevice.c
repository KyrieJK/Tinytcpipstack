#include "../include/netif.h"
#include "../include/ethernet.h"
#include "../include/lib.h"
#include "../include/list.h"
#include "../include/netconfig.h"

/* 本地设备抽象结构存储链表 */
struct list_head net_devices;

extern void veth_poll(void);

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




