#include"fryerP2Server.h"

int initializeServers()
{
    printf("Retrieving server info...\n");

    char name[MAX_NAME_LENGTH] = "";
    char address[MAX_NAME_LENGTH] = "";

    FILE *receiverlist;
    if ((receiverlist = fopen(RECEIVER_LIST,"r")) == NULL)
    {
        perror("Error opening receiver list file.\n");
        return -1;
    }

    //first line is header
    if(fscanf(receiverlist,"%s %s",name,address) < 2)
    {
        fprintf(stderr, "Matched less than two string sequences from header of user list file.\n");
        return -1;
    }

    memset(&name, '\0', (sizeof(char)*MAX_NAME_LENGTH));
    memset(&address, '\0', (sizeof(char)*MAX_NAME_LENGTH));

    int i = 0;
    
    while(fscanf(receiverlist,"%s %s",name,address) != EOF && i<NUM_SERVERS)
    {
        strncpy(serverPairs.entry[i].name, name, (strlen(name)+1));
        strncpy(serverPairs.entry[i].address, address, (strlen(address)+1));
        ++i;
        memset(&name, '\0', (sizeof(char)*MAX_NAME_LENGTH));
        memset(&address, '\0', (sizeof(char)*MAX_NAME_LENGTH));
    }

    if(i == NUM_SERVERS)
    {
        for(int j = 0; j<i;++j)
        {
            printf("%i: %s %s\n",j,serverPairs.entry[j].name, serverPairs.entry[j].address);
        }
        printf("Success.\n");
    }
    else
    {
        fprintf(stderr, "Found %i user entries. Expected %i\n",i,NUM_SERVERS);
    }

    fclose(receiverlist);

    return 0;
}

int initializeUsers()
{
    printf("Retrieving user info...\n");

    char username[MAX_NAME_LENGTH] = "";
    char password[MAX_NAME_LENGTH] = "";

    FILE *userlist;
    if ((userlist = fopen(USER_LIST,"r")) == NULL)
    {
        perror("Error opening user list file.\n");
        return -1;
    }

    //first line is header
    if(fscanf(userlist,"%s %s",username,password) < 2)
    {
        fprintf(stderr, "Matched less than two string sequences from header of user list file.\n");
        return -1;
    }

    memset(&username, '\0', (sizeof(char)*MAX_NAME_LENGTH));
    memset(&password, '\0', (sizeof(char)*MAX_NAME_LENGTH));

    int i = 0;
    
    while(fscanf(userlist,"%s %s",username,password) != EOF && i<NUM_USERS)
    {
        strncpy(userPairs.entry[i].name, username, (strlen(username)+1));
        strncpy(userPairs.entry[i].password, password, (strlen(password)+1));
        ++i;
        memset(&username, '\0', (sizeof(char)*MAX_NAME_LENGTH));
        memset(&password, '\0', (sizeof(char)*MAX_NAME_LENGTH));
    }

    if(i == NUM_USERS)
    {
        for(int j = 0; j<i;++j)
        {
            printf("%i: %s %s\n",j,userPairs.entry[j].name, userPairs.entry[j].password);
        }
        printf("Success.\n");
    }
    else
    {
        fprintf(stderr, "Found %i user entries. Expected %i\n",i,NUM_USERS);
    }

    fclose(userlist);

    return 0;
}

int relayMessages(int senderfd, int receiverfd)
{
    int status = -1;
    int numSent = 1;
    char *cliMsg = NULL;
    char *recvResponse = NULL;

    if((cliMsg = readFromSocket(senderfd)) == NULL)
    {
        return status;
    }

    while(strcmp(cliMsg, "CLOSE") != 0)
    {
        if(sendToSocket(senderfd, cliMsg) != 0)
        {
            status = -1;
            break;
        }

        if(sendToSocket(receiverfd, cliMsg) != 0)
        {
            status = -1;
            break;
        }

        if((numSent % 2) == 0)
        {
            if((recvResponse = readFromSocket(receiverfd)) == NULL)
            {
                status = -1;
                break;
            }
            if(sendToSocket(senderfd, recvResponse) != 0)
            {
                status = -1;
                break;
            }
        }

        free(cliMsg);
        cliMsg = NULL;
        free(recvResponse);
        recvResponse = NULL;

        ++numSent;
        if((cliMsg = readFromSocket(senderfd)) == NULL)
        {
            status = -1;
            break;
        }
    }

    if(cliMsg != NULL)
    {
        if (strcmp(cliMsg, "CLOSE") == 0)
        {
            printf("Sending close message to receiver.\n");
            sendToSocket(receiverfd, cliMsg);
            status = 0;
        }
        free(cliMsg);
    }
    if(recvResponse != NULL)
    {
        free(recvResponse);
    }

    return status;
}

int connectToReceiver(int serverno, int recvPort, int *recvfd, struct sockaddr_in * recvAddress)
{
    if (((*recvfd) = createClientSocket(recvPort, serverPairs.entry[serverno].address, recvfd, recvAddress)) < 0)
    {
        return -1;
    }
    
    printf("Relay is attempting to contact receiving server.\n");
    
    for(int attempts = 0;attempts < 5;++attempts)
    {
        if (connect((*recvfd), (struct sockaddr*) recvAddress, sizeof((*recvAddress))) < 0)
        {
            char msg[64];
            memset(msg,'\0',(sizeof(char)*64)); 
            snprintf(msg,(sizeof(char)*64), "Attempt %i of 5: connection error.\n", (attempts+1));
            perror(msg);
            sleep(2);
        }
        else
        {
            printf("Attempt %i: connection successful.\n", (attempts+1));
            return (*recvfd);
        }
    }

    return -1;
}

int matchReceivers(char* arg)
{
    for(int i = 0; i < NUM_SERVERS; ++i)
    {
        if((strcmp(serverPairs.entry[i].name, arg) == 0) 
                || (strcmp(serverPairs.entry[i].address, arg) == 0))
        {
            return i;
        }
    }
    return -1;
}

int matchCredentials(char* arg1, char* arg2)
{
    for(int i = 0; i < NUM_USERS; ++i)
    {
        if((strcmp(userPairs.entry[i].name, arg1) == 0) 
                && (strcmp(userPairs.entry[i].password, arg2) == 0))
        {
            return 0;
        }
    }
    return -1;
}

int getReceiver(int senderfd, int port, int *recvPort, int *serverno)
{
    char *name = NULL;
    char *input = NULL;
    (*serverno) = -1;
    (*recvPort) = 0;
    int result = -1;

    while(result < 0)
    {
        if((input = readFromSocket(senderfd)) != NULL)
        {
            name = strndup(input, DEFAULT_BUFFER_SIZE);
            sendToSocket(senderfd, name);
            free(input);
            input = NULL;
        }
        else
        {
            return result;
        }
        if((input = readFromSocket(senderfd)) != NULL)
        {
            if (((*recvPort) = atoi(input)) <= 0)
            {
                printf("Invalid port number received: %i\n", (*recvPort));
            }
            //sendToSocket(senderfd, password);
            free(input);
            input = NULL;
        }
        else
        {
            return result;
        }

        if((((*serverno) = matchReceivers(name)) < 0) 
                || ((*recvPort) != (port + 1)))
        {
            sendToSocket(senderfd, "-1");
            (*recvPort) = 0;
            (*serverno) = 0;
        }
        else
        {
            sendToSocket(senderfd, "0");
            printf("Receiving Server: %s Port: %i\n", serverPairs.entry[(*serverno)].name, (*recvPort));
            result = 0;
        }

        free(name);
        name = NULL;
    }

    return result;
}

int authenticate(int senderfd)
{
    char *username = NULL;
    char *password = NULL;
    char *input = NULL;
    int result = -1;

    while(result < 0)
    {
        if((input = readFromSocket(senderfd)) != NULL)
        {
            username = strndup(input, DEFAULT_BUFFER_SIZE);
            sendToSocket(senderfd, username);
            free(input);
            input = NULL;
        }
        else
        {
            return -1;
        }
        if((input = readFromSocket(senderfd)) != NULL)
        {
            password = strndup(input,DEFAULT_BUFFER_SIZE);
            //sendToSocket(senderfd, password);
            free(input);
            input = NULL;
        }
        else
        {
            return -1;
        }

        if((result = matchCredentials(username, password)) < 0)
        {
            sendToSocket(senderfd, "-1");
        }
        else
        {
           sendToSocket(senderfd, "0");
        }
        
    }

    if(username != NULL)
    {
        free(username);
    }
    if(password != NULL)
    {
        free(password);
    }

    return 0;
}

int serverProgram(int senderfd, int port)
{
    int status = -1;
    struct sockaddr_in recvAddress;
    int recvfd = -1;
    int recvPort = 0;
    int recvServer = -1;
    
    if(initializeUsers()<0)
    {
        return status;
    }
    if((status = authenticate(senderfd)) < 0)
    {
        return status;
    }
    if(initializeServers() < 0)
    {
        return status;
    }
    if((status = getReceiver(senderfd, port, &recvPort, &recvServer)) < 0)
    {
        return status;
    }
    if((recvfd = connectToReceiver(recvServer, recvPort, &recvfd, &recvAddress)) < 0)
    {
        sendToSocket(senderfd, "-1");
        return status;
    }
    else
    {
        sendToSocket(senderfd, "0");

        if((status = relayMessages(senderfd, recvfd)) < 0)
        {
            status = -1;
        }

        close(recvfd);
    }

    return status;
}

int serverLoop(int sockfd, int port)
{
    struct sockaddr_in cliaddr;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    
    for(;;)
    {
        memset(&cliaddr, 0, sizeof(struct sockaddr_in));
        
        printf("Waiting for connection...\n");
        
        int senderfd = accept(sockfd, (struct sockaddr*) &cliaddr, &clientlen);

        if (senderfd < 0)
        {
            perror("Error accepting connection.\n");
        }
        else
        {
            printf("Accepted.\n");
            if(serverProgram(senderfd, port) < 0)
            {
                fprintf(stderr, "An error was encountered.\n");
            }
        }

        close(senderfd);
        printf("Connection Closed.\n");
    }

    return EXIT_SUCCESS;
}

int usage(char* str)
{
    fprintf(stderr, "Usage: %s -p port_number\n",str);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    int port = 0;
    if (argc > 2 && argc < 4)
    {
        char* arg;
        int opt = 0;
        while((opt = getopt(argc, argv, ":p:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    port = parsePortNo(optarg);
                    if (port==0)
                    {
                        return usage(argv[0]);
                    }
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0]);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0]);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0]);
           }
        }
    }
    else
    {
        return usage(argv[0]);
    }

    printf("Port: %i.\n", port);

    struct sockaddr_in serveraddr;
    int sockfd = 0;

    if((sockfd = createServerSocket(port, &serveraddr)) < 0)
    {
        return EXIT_FAILURE;
    }

    if(listen(sockfd,50)<0)
    {
        perror("Error setting socket to listen.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    return serverLoop(sockfd,port);
}
