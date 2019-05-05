//
// Created by KyrieJK on 2019/4/22.
//

#ifndef TINYTCPIPSTACK_LIB_H
#define TINYTCPIPSTACK_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>

#include <linux/syscalls.h>

#define gettid() syscall(SYS_gettid)
#define ferr(fmt,args...) fprintf(stderr,fmt,##args)
#define dbg(fmt,args...) ferr("[%d]%s " fmt "\n",(int)(gettid()),__FUNCTION__,##args);

extern int min(int x,int y);
extern int max(int x,int y);
extern void *xmalloc(int);
extern void *xcalloc(int);
#endif //TINYTCPIPSTACK_LIB_H
