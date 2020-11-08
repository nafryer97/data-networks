#ifndef RECEIVER_H
#define RECEIVER_H

#include"fryerP2Common.h"

int hamming(int sockfd, char *arg1, char *arg2);

int serverLoop(int sockfd);

int usage(char *arg);

int main(int argc, char *argv[]);

#endif
