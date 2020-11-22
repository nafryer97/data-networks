#ifndef SENDER_H
#define SENDER_H

#include"fryerSelectiveRepeatCommon.h"

#ifndef SENDER_PROMPT
#define SENDER_PROMPT "P4sender~%"
#endif

int authenticate(int sockfd, struct sockaddr_in *clientaddr, struct user_list *userList);

int main(int argc, char *argv[]);

#endif
