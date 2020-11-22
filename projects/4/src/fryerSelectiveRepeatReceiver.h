#ifndef RECEIVER_H
#define RECEIVER_H

#include"fryerSelectiveRepeatCommon.h"

#ifndef RECEIVER_PROMPT
#define RECEIVER_PROMPT "P4receiver~%"
#endif

#ifndef THREAD_SOCKFD_ERR
#define THREAD_SOCKFD_ERR "Error opening socket."
#endif

#ifndef THREAD_SOCKFDREAD_ERR
#define THREAD_SOCKFDREAD_ERR "Error reading from socket."
#endif

#ifndef THREAD_SOCKFDREAD_SUCC
#define THREAD_SOCKFDREAD_SUCC "Exited after reading from sender."
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

int waitForConfirmation(struct sender_info *sender);

int sendCredentials(const char *name, const char*password, struct sender_info *sender);

int receiverProgram(struct sender_info *sender);

int getCredentials(struct user_info *credentials);

int main(int argc, char *argv[]);

#endif
