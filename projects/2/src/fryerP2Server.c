#include"fryerP2Server.h"

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

int getCredentials(int senderfd)
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

int serverLoop(int sockfd)
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
            initializeUsers();
            getCredentials(senderfd);
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

    return serverLoop(sockfd);
}
