#include "ethernet.h"
#include "arp.h"
#include "lib.h"
#include "list.h"

pthread_mutex_t arp_cache_mutex;/* arp高速缓存表锁 */

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


