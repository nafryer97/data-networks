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

#ifndef MEDIUM_BUFER_SIZE 
#define MEDIUM_BUFFER_SIZE 256
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

#ifndef NUM_USERS
#define NUM_USERS 7
#endif

#ifndef USER_LIST
#define USER_LIST "userList.txt"
#endif

#ifndef SENDER_CONFIRM
#define SENDER_CONFIRM "SAck"
#endif

#ifndef SENDER_ACCEPT
#define SENDER_ACCEPT "Accepted"
#endif

#ifndef RECEIVER_CONFIRM 
#define RECEIVER_CONFIRM "RAck"
#endif

struct user_info 
{
    char name[SMALL_BUFFER_SIZE];
    char password[SMALL_BUFFER_SIZE];
};

struct user_list
{
    struct user_info users[NUM_USERS];
};

void redStdout(const char *msg);

void greenStdout(const char *msg);

void yellowStdout(const char *msg);

void blueStdout(const char *msg);

void magentaStdout(const char *msg);

void cyanStdout(const char *msg);

char* readFromUDPSocket(int sockfd, socklen_t *socklen, struct sockaddr_in *sockaddr);

int sendToUDPSocket(int sockfd, const char* str, struct sockaddr_in *dest);

void removeNewLine(char* str);

char* getInput();

int parsePortNo(char* arg);

int createUDPClientSocket(int port, const char* address, int *sockfd, struct sockaddr_in *cliaddress);

int createUDPServerSocket(int port, struct sockaddr_in *serveraddr);

int usage(char *arg1, char *arg2);

void handleFatalErrorNo(int en, const char *msg);

int handleErrorNoRet(int en, int retval, const char *msg);

void handleErrorNoMsg(int en, const char *msg);

void handleFatalError(const char *msg);

int handleErrorRet(int retval, const char *msg);

void handleErrorMsg(const char *msg);

#endif
