#ifndef CLIENT_H
#define CLIENT_H

#include"fryerP3Common.h"

int clientProgram(int sockfd, struct sockaddr_in serveraddr, char *group_name);

int setup(char *group_name);

int main(int argc, char *argv[]);

#endif
