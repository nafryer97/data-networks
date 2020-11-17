#ifndef CLIENT_H
#define CLIENT_H

#include"fryerP3Common.h"

struct server_info 
{
    struct sockaddr_in *serveraddr;
    int serverfd;
};

int clientProgram(int sockfd, struct sockaddr_in serveraddr, char *group_name);

int setup(char *group_name);

int main(int argc, char *argv[]);

#endif
