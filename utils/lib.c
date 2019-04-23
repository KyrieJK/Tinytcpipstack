//
// Created by JKerving on 2019/4/22.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

int min(int x,int y){
    return x > y ? y : x;
}

int max(int x,int y){
    return x > y ? x : y;
}

void perrx(char *str){
    if(errno){
        perror(str);
    } else{
        fprintf(stderr,"ERROR:%s\n",str);
    }
    exit(EXIT_FAILURE);
}

/**
 * malloc函数封装
 * @param size
 * @return
 */
void *xmalloc(int size){
    void *p=malloc(size);
    if(!p){
        perrx("malloc");
    }
    return p;
}

/**
 * calloc函数封装
 * @param size
 * @return
 */
void *xcalloc(int size){
    void *p=calloc(1,size);
    if(!p){
        perrx("calloc");
    }
    return p;
}

/**
 * 字符串转换为IP地址
 * @param str
 * @param ip
 * @return
 */
int str2ip(char *str, unsigned int *ip)
{
    unsigned int a, b, c, d;
    if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
        return -1;
    if (a > 255 || b > 255 || c > 255 || d > 255)
        return -1;
    *ip = a | (b << 8) | (c << 16) | (d << 24);
    return 0;
}

/**
 * 解析IP地址中的端口号
 * @param str
 * @param addr
 * @param nport
 * @return
 */
int parse_ip_port(char *str, unsigned int *addr, unsigned short *nport){
    char *port=NULL;
    int n=0;
    if((port=strchr(str,':'))!=NULL){
        nport = (unsigned short*)malloc(sizeof(unsigned short));
        n = atoi(&port[1]);
        *nport = n;
    }

    if(str2ip(str,addr)<0)
        return -1;

    return 0;
}