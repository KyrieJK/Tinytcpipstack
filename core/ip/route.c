//
// Created by 张家望 on 2019/12/31.
//
#include "../../include/route.h"
#include "../../include/lib.h"
#include "../../include/list.h"
#include "../../include/ip.h"
#include "../../include/netif.h"
#include "../../include/netconfig.h"
#include "../../include/icmp.h"

static LIST_HEAD(rt_head);


/* 用于在当前路由表搜索符合条件的路由表项 */
struct rt_entry *rt_lookup(unsigned int ipaddr) {
    struct rt_entry *rt;
    list_for_each_entry(rt, &rt_head, rt_list) {
        if ((rt->rt_netmask & ipaddr) == (rt->rt_netmask & rt->rt_net))
            return rt;
    }
    return NULL;
}

/* 创建一条路由表项 */
struct rt_entry *rt_alloc(unsigned int net, unsigned int netmask, unsigned int gateway, int metric, unsigned int flags,
                          struct net_device *dev) {
    struct rt_entry *rt;
    rt = (struct rt_entry *) malloc(sizeof(*rt));
    rt->rt_net = net;
    rt->rt_netmask = netmask;
    rt->rt_gateway = gateway;
    rt->rt_metric = metric;
    rt->rt_flags = flags;
    rt->rt_dev = dev;
    list_init(&rt->rt_list);
    return rt;
}

/* 添加路由表项 */
void rt_add(unsigned int net, unsigned int netmask, unsigned int gw, int metric, unsigned int flags,
            struct net_device *dev) {
    struct rt_entry *rt, *rte;
    struct list_head *l;
    rt = rt_alloc(net, netmask, gw, metric, flags, dev);
    l = &rt_head;
    list_for_each_entry(rte, &rt_head, rt_list) {
        if (rt->rt_netmask >= rte->rt_netmask) {
            l = &rte->rt_list;
            break;
        }
    }

    list_add_tail(&rt->rt_list, l);

}

/* 路由表初始化 */
void rt_init(void) {
    rt_add(LOCALNET(loop), loop->net_mask, 0, 0, RT_LOCALHOST, loop);
    rt_add(veth->net_ipaddr, 0xffffffff, 0, 0, RT_LOCALHOST, loop);
    rt_add(LOCALNET(veth), veth->net_mask, 0, 0, RT_NONE, veth);
    rt_add(0, 0, tap->dev.net_ipaddr, 0, RT_DEFAULT, veth);
}

int rt_input(struct pk_buff *pkb){
    struct ip *iphdr=pkb2ip(pkb);
    struct rt_entry *rt=rt_lookup(iphdr->daddr);
    if (rt==NULL){
#ifndef CONFIG_TOP1
        /**
         * If a router cannot forward a packet because it has no routes
		 * at all (including no default route) to the destination
		 * specified in the packet, then the router MUST generate a
		 * Destination Unreachable, Code 0 (Network Unreachable) ICMP
		 * message.
         */
         ip_hton(iphdr);
         icmp_send(ICMP_T_DESTUNREACH,ICMP_HOST_UNREACH,0,pkb);
#endif
         free_pkb(pkb);
         return -1;
    }
    pkb->pk_rtdst=rt;
    return 0;
}

/*根据路由表查找路由项，确定pkb目的地址以及根据路由项所在netdevice的ip地址设置ip包src address*/
int rt_output(struct pk_buff *pkb) {
    struct ip *iphdr = pkb2ip(pkb);
    struct rt_entry *rt = rt_lookup(iphdr->daddr);
    if (!rt) {
        return -1;
    }
    pkb->pk_rtdst = rt;
    iphdr->saddr = rt->rt_dev->net_ipaddr;
    return 0;
}

void rt_traverse(void) {
    struct rt_entry *rt;
    if (list_empty(&rt_head))
        return;
    printf("Destination      Gateway     Genmask       Metric Iface\n");
    list_for_each_entry(rt, &rt_head, rt_list) {
        //遍历每个rt_entry
    }
}