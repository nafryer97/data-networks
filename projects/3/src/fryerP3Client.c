#include"fryerP3Client.h"

void *waitForMessage(void *info_ptr)
{
    printf("Listening for messages from server...\n");

    const struct server_info *server_ptr = (struct server_info *)info_ptr;
    const struct sockaddr_in serveraddr = (*(*server_ptr).serveraddr);
    const int serverfd = (*server_ptr).serverfd;
    char *serverMsg = NULL;
    
    for(;;)
    {
        if((serverMsg = readFromUDPSocket(serverfd,0,NULL)) != NULL)
        {
            printf("Message from server: %s\n", serverMsg);
            if (strcmp(serverMsg, MSG_CLEARALL) == 0)
            {
                printf("Message thread is returning.\n");
                break;
            }
            else if (strcmp(serverMsg, CLIENT_REMOVED) == 0)
            {
                printf("Message thread is returning.\n");
                break;
            }
            free(serverMsg);
            serverMsg=NULL;
        }
    }
    
    if (serverMsg != NULL)
    {
        free(serverMsg);
    }

    return NULL;
}

int clientProgram(int sockfd, struct sockaddr_in serveraddr, char *group_name)
{
    printf("Please enter a message to send to server.\n\"JOIN\" to join %s\n\"QUIT\"to leave %s\n\"EXIT\" to end program.\n",group_name,group_name);

    char *input = getInput(SMALL_BUFFER_SIZE);
    socklen_t server_len = sizeof(serveraddr);
    
    struct server_info *server_ptr = malloc(sizeof(struct server_info));
    memset(server_ptr, 0, sizeof(struct server_info));
    (*server_ptr).serveraddr = malloc(sizeof(struct sockaddr_in));
    memcpy((*server_ptr).serveraddr,&serveraddr,sizeof(struct sockaddr_in));
    (*server_ptr).serverfd = sockfd;

    pthread_t thread_id;
    memset(&thread_id,0,sizeof(pthread_t));

    while(strcmp(input, "EXIT") != 0)
    {
        if(strcmp(input, "JOIN") == 0)
        {
            sendToUDPSocket(sockfd,input,&serveraddr);
            if(pthread_create(&thread_id,NULL,waitForMessage,(void *)server_ptr) != 0)
            {
                perror("Error creating thread.\n");
                break;
            }
        }
        else if(strcmp(input, "QUIT") == 0)
        {
            char exit_status[SMALL_BUFFER_SIZE];
            sendToUDPSocket(sockfd,input,&serveraddr);
            if(pthread_cancel(thread_id) != ESRCH)
            {
                if(pthread_join(thread_id,NULL) == 0)
                {
                    printf("Listening thread successfully cancelled.\n");
                }
            }
        }
        else
        {
            printf("Unknown command: %s\n",input);
        }

        free(input);
        input = NULL;
    
        printf("Please enter a message to send to server.\n\"JOIN\" to join %s\n\"QUIT\"to leave %s\n\"EXIT\" to end program.\n",group_name,group_name);
        
        input = getInput(DEFAULT_BUFFER_SIZE);
    }
    
    free(input);
    free((*server_ptr).serveraddr);
    free(server_ptr);

    sendToUDPSocket(sockfd,"QUIT",&serveraddr);
    if(pthread_cancel(thread_id) != ESRCH)
    {
        if(pthread_join(thread_id,NULL) == 0)
        {
            printf("Listening thread successfully cancelled.\n");
        }
    }
    
    return 0;
}

int setup(char *group_name)
{
    printf("Please enter a group name:\n");

    char *input = getInput(SMALL_BUFFER_SIZE);

    strncpy(group_name,input,SMALL_BUFFER_SIZE);

    free(input);
}

int main(int argc, char* argv[])
{
    char client_usage[] = "[-s SERVER_IP_ADDRESS] [-p PORT_NUMBER]";
    int port = 0;
    char* address;
    
    if (argc > 2 && argc < 6)
    {
        char* args[2];
        int opt = 0;
        while((opt = getopt(argc, argv, ":s:p:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    args[0] = optarg;
                    port = parsePortNo(args[0]);
                    if (!port)
                    {
                        return usage(argv[0],client_usage);
                    }
                    break;
                case 's':
                    args[1] = optarg;
                    address = args[1];
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0], client_usage);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0], client_usage);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0], client_usage);
           }
        }
    }
    else
    {
        return usage(argv[0], client_usage);
    }

    struct sockaddr_in serveraddr;
    int sockfd;

    if ((sockfd = createUDPClientSocket(port, address, &sockfd, &serveraddr)) < 0)
    {
        return EXIT_FAILURE;
    }

    char group_name[SMALL_BUFFER_SIZE] = "";

    setup(group_name);

    printf("Group name: %s\n",group_name);

    if(clientProgram(sockfd, serveraddr, group_name) == 0)
    {
        close(sockfd);
        printf("Goodbye.\n");
        return EXIT_SUCCESS;
    }
    else
    {
        close(sockfd);
        return EXIT_FAILURE;
    }
}
