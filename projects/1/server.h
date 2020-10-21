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

#ifndef DEFAULT_CLI_PORT
#define DEFAULT_CLI_PORT 50051
#endif

#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 1024
#endif

#ifndef DEFAULT_MIN_PORT
#define DEFAULT_MIN_PORT 50000
#endif

#ifndef DEFAULT_MAX_PORT
#define DEFAULT_MAX_PORT 59999
#endif

int checkCurrency(char* input);

char* checkPassword(int currency, char* password);

void removeNewLine(char* str);

int getPortNumCli();

int openSocket();

int bindPort(int socketfd, int cliPort, struct sockaddr_in* sa);

void processInput(int clientfd, char* currency, char* password);

void readCurrency(int clientfd, char* currency);

void readPassword(int clientfd, char* password);

void currencyProgram(int clientfd);

#endif
