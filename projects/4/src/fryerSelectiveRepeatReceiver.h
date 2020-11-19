#ifndef RECEIVER_H
#define RECEIVER_H

#include"fryerSelectiveRepeatCommon.h"

#ifndef RECEIVER_PROMPT
#define RECEIVER_PROMPT "P4receiver~%"
#endif

struct sender_info 
{
    int port;
    char addrStr[SMALL_BUFFER_SIZE];
    int sockfd;
    struct sockaddr_in serveraddr;
    char inpFileName[SMALL_BUFFER_SIZE];
    char outFileName[SMALL_BUFFER_SIZE];
};

void *listenToSender(void *info);

int senderProgram(struct user_info *credentials);

int getCredentials(struct user_info *credentials);

int main(int argc, char *argv[]);

#endif
