#ifndef SERVER_H
#define SERVER_H

#include<netinet/in.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<errno.h>

int checkCurrency(char* input);

char* checkPassword(int currency, char* password);

#ifndef CLI_PORT
#define CLI_PORT 50051
#endif

#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 1024
#endif

#endif
