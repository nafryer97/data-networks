#ifndef SENDER_H
#define SENDER_H

#include"fryerSelectiveRepeatCommon.h"

#ifndef SENDER_PROMPT
#define SENDER_PROMPT "P4sender~%"
#endif

#ifndef SENDER_USAGE
#define SENDER_USAGE "[ -p PORT_NUMBER ] [ -w WINODW_SIZE ]\n\nMust use a port between 50000 and 59999 (inclusive).\nUndergraduates need only implement window size of 1.\n"
#endif

int authenticate(int sockfd, struct sockaddr_in *clientaddr, struct user_list *userList);

int checkFile(char *fileName, FILE *inpFile);

int getFileInfo(int sockfd, struct sockaddr_in *clientaddr, char *fileName, FILE *inpFile);

int getReceiverInfo(int sockfd, struct sockaddr_in *clientaddr, struct user_list *userList, int windowSize);

int main(int argc, char *argv[]);

void setup(int port, int *sockfd, int *numUsers, struct sockaddr_in *addr, struct user_list *userList);

void printUserList(struct user_list *userList, int i);

int createUserList(struct user_list *userList);

void *thr_createUserList(void *info);

#endif
