#ifndef RECEIVER_H
#define RECEIVER_H

#include"fryerSelectiveRepeatCommon.h"

#ifndef RECEIVER_USAGE
#define RECEIVER_USAGE "[-s SERVER_IP_ADDRESS] [-p PORT_NUMBER] [-i INPUT_FILE] [-o OUTPUT_FILE]\n\nPort Number must be between 50000 and 59999.\nInput file name will be sent to the server. Output file will be created locally."
#endif

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


int main(int argc, char *argv[]);

void setup(int port, char *address, struct user_info *credentials, struct sender_info *sender, int *windowSize);

int confirmTransfer(struct sender_info *sender);

int sendFileInfo(char *inpFile, struct sender_info *sender);

int waitForConfirmation(struct sender_info *sender);

int sendCredentials(const char *name, const char*password, struct sender_info *sender);

int getCredentials(struct user_info *credentials);

//void *listenToSender(void *info);
#endif
