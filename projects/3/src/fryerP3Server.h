#ifndef SERVER_H
#define SERVER_H

#include"fryerP3Common.h"

#ifndef MAX_GROUP_SIZE
#define MAX_GROUP_SIZE 20
#endif

struct group_info
{
    struct sockaddr_in *server_addr;
    int serverfd;
    struct sockaddr_in *client_addresses[MAX_GROUP_SIZE];
    char *group_name;
    int group_size;
    int num_clients;
    int flags[MAX_GROUP_SIZE];
};

int compareAddresses(struct sockaddr_in *addr1, struct sockaddr_in *addr2);

int addToGroup(struct group_info *info, struct sockaddr_in *new_addr);

int removeFromGroup(struct group_info *info, struct sockaddr_in *client_addr);

void printGroupInfo(struct group_info *info);

int initializeSessionInfo(struct group_info *info);

void *listenToServerSocket();

int serverProgram(struct group_info *info);

int setup();

int main(int argc, char *argv[]);

#endif
