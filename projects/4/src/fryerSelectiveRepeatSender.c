#include"fryerSelectiveRepeatSender.h"

int authenticate(int sockfd, struct sockaddr_in *clientaddr, struct user_list *userList)
{
    char errMsg[SMALL_BUFFER_SIZE] = "";
    char *name = NULL;
    char *password = NULL;
    socklen_t socklen = sizeof(struct sockaddr_in);

    for(;;)
    {
        memset(clientaddr, 0, sizeof(struct sockaddr_in));
        printf("Waiting for username...\n");

        if((name = readFromUDPSocket(sockfd, &socklen, clientaddr)) == NULL)
        {
            return handleErrorRet(-1, "Error waiting for username");        
        }

        if(sendToUDPSocket(sockfd, SENDER_CONFIRM, clientaddr) != 0)
        {
            free(name);
            return handleErrorRet(-1, "Error sending confirmation message");
        }

        printf("Waiting for password...\n");

        if((password = readFromUDPSocket(sockfd, &socklen, clientaddr)) == NULL)
        {
            free(name);
            return handleErrorRet(-1, "Error waiting for password");        
        }

        if(sendToUDPSocket(sockfd, SENDER_CONFIRM, clientaddr) != 0)
        {
            free(name);
            free(password);
            return handleErrorRet(-1, "Error sending confirmation message");
        }

        printf("Verifying...\n");

        for(int i = 0; i < NUM_USERS; ++i)
        {
            if((strcmp(name,(*userList).users[i].name) == 0) && (strcmp(password, (*userList).users[i].password) == 0))
            {
                free(name);
                free(password);
                return(i);
            }
        }

        snprintf(errMsg, SMALL_BUFFER_SIZE, "Auth failed. Name: %s Password: %s", name, password);
        redStdout(errMsg);

        free(name);
        free(password);
        
        if(sendToUDPSocket(sockfd, "Auth failed", clientaddr) != 0)
        {
            return handleErrorRet(-1, "Error sending auth message");
        }

    }
}

int main(int argc, char* argv[])
{
    char server_usage[] = "[-p PORT_NUMBER] [-w WINODW_SIZE]";

    int port = 0;
    int window_size = 0;

    if (argc == 5)
    {
        int opt = 0;
        while((opt = getopt(argc, argv, ":p:w:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    
                    if ((port = parsePortNo(optarg)) == 0)
                    {
                        return usage(argv[0],server_usage);
                    }
                    break;
                case 'w':
                    window_size = atoi(optarg);
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0],server_usage);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0],server_usage);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0],server_usage);
           }
        }
    }
    else
    {
        return usage(argv[0],server_usage);
    }

    yellowStdout("Retrieving user info...");

    struct user_list userList;
    FILE *userFile;

    if ((userFile = fopen(USER_LIST,"r")) == NULL)
    {
        int errnum = errno;
        handleFatalErrorNo(errnum, "Error opening receiver list file");
    }

    char name[SMALL_BUFFER_SIZE];
    char password[SMALL_BUFFER_SIZE];
    int i;

    for(i=0;i<NUM_USERS;++i)
    {
        if(fscanf(userFile,"%s %s",name,password)!=EOF)
        {
            strncpy(userList.users[i].name, name, SMALL_BUFFER_SIZE);
            strncpy(userList.users[i].password, password, SMALL_BUFFER_SIZE);
        }
        else
        {
            break;
        }
    }

    if(i == NUM_USERS)
    {
        for(int j = 0; j<i;++j)
        {
            printf("%i: %s %s\n",j,userList.users[j].name,userList.users[j].password);
        }
        greenStdout("Success.");
    }
    else
    {
        char errMsg[SMALL_BUFFER_SIZE] = "";
        snprintf(errMsg, SMALL_BUFFER_SIZE, "Found %i user entries. Expected %i",i,NUM_USERS);
        redStdout(errMsg);
    }

    fclose(userFile);

    int sockfd = 0;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;

    if((sockfd=createUDPServerSocket(port, &addr))<0)
    {
        handleFatalError("Exiting...");
    }

    int useri = authenticate(sockfd, &clientaddr, &userList);

    if (useri < 0)
    {
        close(sockfd);
        handleFatalError("Exiting...");
    }

    char accept[SMALL_BUFFER_SIZE] = "";
    snprintf(accept, SMALL_BUFFER_SIZE, "%s:%i", SENDER_ACCEPT,window_size);

    //printf("Sending message \"%s\"\n",accept);

    if(sendToUDPSocket(sockfd, accept, &clientaddr) != 0)
    {
        close(sockfd);
        handleFatalError("Exiting...");
    }

    char welcome[SMALL_BUFFER_SIZE] = "";

    snprintf(welcome, SMALL_BUFFER_SIZE,"Welcome %s",userList.users[useri].name);

    greenStdout(welcome);

    close(sockfd);

    return EXIT_SUCCESS;
}
