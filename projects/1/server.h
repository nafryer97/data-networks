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

/*
 * Checks to see if the input string matches one of the given currency strings.
 * Returns the index of the string in the string array if there is a match.
 */
int checkCurrency(char* input);

/*
 * Checks to see if the given password matches the password used for the given currency.
 * If there is a match it returns the bitcoin value of that currency.
 */
char* checkPassword(int currency, char* password);

/*
 * Removes the first occurrence of a newline character in a string and replaces it with a 
 * nul character.
 */
void removeNewLine(char* str);

/*
 * gets the port number from the user using stdin
 */
int getPortNumCli();

/*
 * opens a berkeley internet socket
 */
int openSocket();

/*
 * binds the open socket to the given port
 */
int bindPort(int socketfd, int cliPort, struct sockaddr_in* sa);

/*
 * processes user input sent from the client. calls checkCurrency and checkPassword in order. sends the output 
 * value returned from checkPassword to the client.
 */
void processInput(int clientfd, char* currency, char* password);

/*
 * waits 30 seconds for the client to send an input string, or times out after 30 secs and returns a null pointer.
 */
char* readCurrency(int clientfd);

/*
 * waits 30 seconds for the client to send an input string, or times out after 30 seconds and returns a null pointer
 */
char* readPassword(int clientfd);

/*
 * coordinates the actions of readCurrency, readPassword, and processInput to perform the program.
 */
int currencyProgram(int clientfd);

#endif
