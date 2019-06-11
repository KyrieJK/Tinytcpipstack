#ifndef __WAIT_H
#define __WAIT_H

#include "lib.h"
#include <pthread.h>

struct wait_simulation{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int dead;
    int notified;
    int sleep;
};

/**
 * 模拟线程初始化
 */
static void wait_init(struct wait_simulation *w){
    pthread_cond_init(&w->cond,NULL);
    pthread_mutex_init(&w->mutex,NULL);
    w->dead = 0;
    w->notified = 0;
    w->sleep = 0;
}

static int wake_up(struct wait_simulation *w){
    pthread_mutex_lock(&w->mutex);
    if(w->dead){
        pthread_mutex_unlock(&w->mutex);
        return -(w->dead);
    }
    if(w->notified == 0){
        w->notified = 1;
        if(w->sleep)
            pthread_cond_signal(&w->cond);
    }
}

static void wait_exit(struct wait_simulation *w){
    pthread_mutex_lock(&w->mutex);
    if(w->dead){
        pthread_mutex_unlock(&w->mutex);
    }
    w->dead = 1;
    if(w->sleep){
        pthread_cond_broadcast(&w->cond);//唤醒所有等待线程
    }
}

/**
 * 线程置为wait状态
 */
static int sleep_on(struct wait_simulation *w){
    pthread_mutex_lock(&w->mutex);
    if(w->dead){
        pthread_mutex_unlock(&w->mutex);
        return -(w->dead);
    }
    w->sleep = 1;
    if(w->notified ==0){
        pthread_cond_wait(&w->cond,&w->mutex);
    }
    w->notified = 0;
    w->sleep = 0;
}