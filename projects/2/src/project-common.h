#ifndef PROJECT_COMMON_H
#define PROJECT_COMMON_H

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>

#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 1024
#endif

#ifndef DEFAULT_MIN_PORT
#define DEFAULT_MIN_PORT 50000
#endif

#ifndef DEFAULT_MAX_PORT
#define DEFAULT_MAX_PORT 59999
#endif

#endif


