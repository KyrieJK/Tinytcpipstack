//
// Created by JKerving on 2019/4/22.
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

int min(int x,int y);
int max(int x,int y);
#endif //TINYTCPIPSTACK_LIB_H
