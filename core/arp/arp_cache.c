#include "ethernet.h"
#include "arp.h"
#include "lib.h"
#include "list.h"

#define arp_cache_head (&arp_cache[0])
#define arp_cache_end (&arp_cache[ARP_CACHE_SZ])

pthread_mutex_t arp_cache_mutex;/* arp高速缓存表锁 */

static struct arpentry arp_cache[ARP_CACHE_SZ];/* ARP高速缓存表项容量 */

#ifndef PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_NORMAL PTHREAD_MUTEX_TIMED_NP /* 当一个线程加锁后，其余请求锁的线程形成等待队列，在解锁后按优先级获取锁 */
#endif

/*
    缓存表锁的动态初始化
*/
static inline void arp_cache_lock_init(void){
    pthread_mutexattr_t attr;
    if(pthread_mutexattr_init(&attr) != 0){
        perror("pthread_mutexattr_init");
    }
    if(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_NORMAL) != 0){
        perror("pthread_mutexattr_settype");
    }
    if(pthread_mutex_init(&arp_cache_mutex,&attr) != 0){
        perror("pthread_mutex_init");
    }
}

static inline void arp_cache_lock(void){
    pthread_mutex_lock(&arp_cache_mutex);
}

static inline void arp_cache_unlock(void){
    pthread_mutex_unlock(&arp_cache_mutex);
}

void arp_cache_init(void){
    int i;
    for(int i=0;i<ARP_CACHE_SZ;i++){
        arp_cache[i].ae_state = ARP_FREE;
    }
    arp_cache_lock_init();
    printf("ARP CACHE INIT");
}

struct arpentry *arp_alloc(void){
    struct arpentry *ae;

    arp_cache_lock();


    static int entry_id=0;
    int i;
    /* 循环找到arp高速缓存表中的空位(FREE) */
    for(i=0;i<ARP_CACHE_SZ;i++){
        if(arp_cache[entry_id].ae_state == ARP_FREE){
            break;
        }
        entry_id = (entry_id+1)%ARP_CACHE_SZ;
    }

    if(i>=ARP_CACHE_SZ){
        printf("arp cache is full");
        arp_cache_unlock();
        return NULL;
    }

    ae = &arp_cache[entry_id];
    ae->ae_dev=NULL;
    ae->ae_retry = ARP_REQ_ATTEMPT;
    ae->ae_ttl = ARP_WAITTIME;
    ae->ae_state = ARP_PENDING;
    ae->ae_pro = ETHERNET_TYPE_IP;
    list_init(&ae->ae_list);

    entry_id = (entry_id+1) % ARP_CACHE_SZ;

    arp_cache_unlock;
}

int arp_insert(struct net_device *nd,unsigned short pro,unsigned int ipaddr,unsigned char *hwaddr){
    struct arpentry *ae;
    ae = arp_alloc();
    ae->ae_dev = nd;
    ae->ae_pro = pro;
    ae->ae_ttl = ARP_TIMEOUT;
    ae->ae_state = ARP_RESOLVED;
    hwacpy(ae->ae_hwaddr,hwaddr);
    return 0;
}

struct arpentry *arp_lookup(unsigned short pro,unsigned int ipaddr){
    struct arpentry *ae,*ret = NULL;
    arp_cache_lock();

    for(ae = arp_cache_head;ae<arp_cache_end;ae++){
        if(ae->ae_state == ARP_FREE){
            continue;
        }
        if(ae->ae_pro == pro && ae->ae_ipaddr == ipaddr){
            ret = ae;
            break;
        }
    }

    arp_cache_unlock();
    return ret;
}

struct arpentry *arp_lookup_resolv(unsigned short pro,unsigned int ipaddr){
    struct arpentry *ae;
    ae = arp_lookup(pro,ipaddr);
    if(ae && ae->ae_state == ARP_RESOLVED)
        return ae;
    return NULL;
}

void arp_queue_send(struct arpentry *ae){
    struct pk_buff *pkb;
    while (!list_empty(&ae->ae_list)){
        pkb = list_first_entry(&ae->ae_list,struct pk_buff,pk_list);
        list_del(ae->ae_list.next);
        printf("send pending packet");
        netdev_tx(ae->ae_dev,pkb,pkb->pk_len-ETH_HRD_SZ,pkb->pk_protocol,ae->ae_hwaddr);
    }   
}

void arp_queue_drop(struct arpentry *ae){
    struct pk_buff *pkb;
    while(!list_empty(&ae->ae_list)){
        pkb = list_first_entry(&ae->ae_list,struct pk_buff,pk_list);
        list_del(ae->ae_list.next);
        printf("drop pending packet");
        free_pkb(pkb);
    }
}

/* arp请求计时器 */
void arp_timer(int delta){
    struct arpentry *ae;
    arp_cache_lock();
    for(ae = arp_cache_head;ae<arp_cache_end;ae++){
        if(ae->ae_state == ARP_FREE)
            continue;
        
        ae->ae_ttl -=delta;
        if(ae->ae_ttl<=0){
            if((ae->ae_state == ARP_PENDING && --ae->ae_retry < 0) || ae->ae_state == ARP_RESOLVED){
                if(ae->ae_state == ARP_PENDING){
                    arp_queue_drop(ae);
                }
                ae->ae_state = ARP_FREE;
            }else {
                ae->ae_ttl = ARP_WAITTIME;
                arp_cache_unlock();
                arp_request(ae);
                arp_cache_lock();
            }
        }
    }
    arp_cache_unlock();
}

static const char *__arpstate[]={
    NULL,
    "FREE",
    "PENDING",
    "RESOLVED"
};

static const char *arpstate(struct arpentry *ae){
    return __arpstate[ae->ae_state];
}

static struct arpentry *arp_cache_freespace(void){
    struct arpentry *ae;
    for(ae = arp_cache_head;ae<arp_cache_end;ae++){
        if(ae->ae_state == ARP_FREE){
            return ae;
        }
    }
    return NULL;
}

void arp_cache_traverse(void){
    struct arpentry *ae;
    arp_cache_lock();
    for(ae = arp_cache_head;ae<arp_cache_end;ae++){
        if(ae->ae_state == ARP_FREE)
            continue;
        printf("HWaddress               Address");
        printf("%s %d",arpstate(ae),((ae->ae_ttl < 0) ? 0 : ae->ae_ttl));
        printf(mac_to_p,mac_addr(ae->ae_hwaddr));
        printf(IPFMT,ipfmt(ae->ae_ipaddr));
    }
    arp_cache_unlock();
}


