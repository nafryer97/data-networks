#ifndef CLIENT_H
#define CLIENT_H

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<string.h>
#include<arpa/inet.h>
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


/*
 *  reads output from the server after the connection. returns an int indicating
 *  success or failure
 */
int readFromServer(int clisoc);

/*
 * sends str to server using socket. returns an int indicating success or failure.
 */
int sendToServer(int clisoc, char* str);

/*
 * coordinates getting input from the user and sending/receing from server. returns an int 
 * indicating success or failure.
 */
int currencyProgram(int clisoc, int* errsv);

/*
 * replaces the first occurrence of a newline char with the nul character
 */
void removeNewLine(char* str);

/*
 * prompts the user for an ip address from the command line
 */
char* getServerAddressCli();

/*
 * prompts the user for a port number from the command line
 */
int getPortNumCli();

#endif
