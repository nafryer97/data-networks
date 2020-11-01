#ifndef SENDER_H
#define SENDER_H

#include"project-common.h"

int readFromServer(int sockfd);

int sendToServer(int sockfd, char* str);

void removeNewLine(char* str);

char* getInput();

int usage();

int main(int argc, char* argv[]);

#endif
