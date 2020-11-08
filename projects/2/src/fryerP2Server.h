#ifndef SERVER_H
#define SERVER_H

#include"project-common.h"

#define USER_LIST "userList.txt"
#define RECEIVER_LIST "receiverList.txt"
#define NUM_USERS 6
#define NUM_SERVERS 7
#define MAX_NAME_LENGTH 64

struct userInfo
{
    char name[MAX_NAME_LENGTH];
    char password[MAX_NAME_LENGTH];
};

struct serverInfo
{
    char name[MAX_NAME_LENGTH];
    char address[MAX_NAME_LENGTH];
};

struct users
{
    struct userInfo entry[NUM_USERS];
};

struct servers
{
    struct serverInfo entry[NUM_SERVERS];
};

struct users userPairs;

struct servers serverPairs;

int initializeServers();

int initalizeUsers();

int relayMessages(int senderfd, int receiverfd);

int connectToReceiver(int serverno, int recvPort, int *recvfd, struct sockaddr_in *recvAddress);

int matchReceivers(char *arg);

int matchCredentials(char *arg1, char *arg2);

int getReceiver(int senderfd, int port, int *recvPort, int *serverno);

int authenticate(int senderfd);

int serverProgram(int sockfd, int port);

int serverLoop(int sockfd, int port);

int usage(char *str);

int main(int argc, char *argv[]);

#endif
