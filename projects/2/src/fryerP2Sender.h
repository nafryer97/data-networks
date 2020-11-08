#ifndef SENDER_H
#define SENDER_H

#include"fryerP2Common.h"

int sendMessages(int serverfd);

int contactReceiver(int sockfd);

int authenticate(int sockfd);

int senderProgram(int sockfd);

int usage();

int main(int argc, char* argv[]);

#endif
