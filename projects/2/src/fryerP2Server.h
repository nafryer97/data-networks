#ifndef SERVER_H
#define SERVER_H

#include"project-common.h"

#define USER_LIST "userList.txt"
#define RECEIVER_LIST "receiverList.txt"
#define NUM_USERS 6
#define NUM_SERVERS 6
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

int initalizeUsers();

int matchCredentials(char *arg1, char *arg2);

int serverLoop(int sockfd);

int usage(char *str);

int main(int argc, char *argv[]);

#endif
