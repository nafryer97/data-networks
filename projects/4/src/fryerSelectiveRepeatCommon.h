#ifndef FRYERP4_COMMON_H
#define FRYERP4_COMMON_H

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>
#include<netdb.h>
#include<pthread.h>

#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 1024
#endif

#ifndef SMALL_BUFFER_SIZE
#define SMALL_BUFFER_SIZE 64
#endif

#ifndef DEFAULT_MIN_PORT
#define DEFAULT_MIN_PORT 50000
#endif

#ifndef DEFAULT_MAX_PORT
#define DEFAULT_MAX_PORT 59999
#endif

#ifndef ERR_MAX_CLIENTS
#define ERR_MAX_CLIENTS "Group is full"
#endif

#ifndef NUM_USERS
#define NUM_USERS 7
#endif

#ifndef USER_LIST
#define USER_LIST "userList.txt"
#endif

struct user_list
{
    char names[NUM_USERS][SMALL_BUFFER_SIZE];
    char passwords[NUM_USERS][SMALL_BUFFER_SIZE];
};

struct user_info 
{
    char name[SMALL_BUFFER_SIZE];
    char password[SMALL_BUFFER_SIZE];
};

char* readFromUDPSocket(int sockfd, socklen_t *socklen, struct sockaddr_in *sockaddr);

int sendToUDPSocket(int sockfd, char* str, struct sockaddr_in *dest);

void removeNewLine(char* str);

char* getInput();

int parsePortNo(char* arg);

int createUDPClientSocket(int port, const char* address, int *sockfd, struct sockaddr_in *cliaddress);

int createUDPServerSocket(int port, struct sockaddr_in *serveraddr);

int usage(char *arg1, char *arg2);

#endif
