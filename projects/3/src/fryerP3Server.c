#include"fryerP3Server.h"

int compareAddresses(struct sockaddr_in *addr1, struct sockaddr_in *addr2)
{
    int errcode = 0;
    char host1[NI_MAXHOST];
    
    if((errcode = getnameinfo((struct sockaddr *) addr1,sizeof(*addr1),host1,NI_MAXHOST,NULL,0,NI_DGRAM|NI_NUMERICHOST))<0)
    {
        fprintf(stderr, "Error getting host1 info: %s\n", gai_strerror(errcode));
        return -1;
    }

    char host2[NI_MAXHOST];
    
    if((errcode = getnameinfo(((struct sockaddr *)addr2),sizeof((*addr2)),host2,NI_MAXHOST,NULL,0,NI_DGRAM|NI_NUMERICHOST))<0)
    {
        fprintf(stderr, "Error getting host2 info: %s\n", gai_strerror(errcode));
        return -1;
    }   
    
    printf("Comparing hosts: %s %s\n",host1,host2);
    
    if (strcmp(host1, host2) == 0)
    {
        printf("Found a match: %s %s\n",host1,host2);
        return 1;
    }
    
    
    return 0;
}

int checkAddresses(struct group_info *info, struct sockaddr_in *addr)
{
    int result = 0;
    for(int i = 0; i < (*info).group_size; ++i)
    {
        if((*info).flags[i]==0)
        {
            printf("Client address index %i is empty.\n",i);
        }
        else
        {
            if((result = compareAddresses(addr, (*info).client_addresses[i])) == 1)
            {
                return i;
            }
            else if (result == -1)
            {
                return -1;
            }
        }
    }

    return result;
}

int addToGroup(struct group_info *info, struct sockaddr_in *new_addr) 
{
    
    if((*info).num_clients >= (*info).group_size)
    {
        printf("%s\n", ERR_MAX_CLIENTS);
        sendToUDPSocket((*info).serverfd,ERR_MAX_CLIENTS,new_addr);
    }
    else
    {
        int compare = checkAddresses(info,new_addr);

        if (compare > -1)
        {
            printf("%s\n", ERR_CLIENT_EXISTS);
            sendToUDPSocket((*info).serverfd,ERR_CLIENT_EXISTS,new_addr);
        }
        else
        {
            for(int i = 0; i < (*info).group_size; ++i)
            {
                if((*info).flags[i] == 0)
                {
                    memcpy((*info).client_addresses[i],new_addr,sizeof((*new_addr)));
                    ++(*info).num_clients;
                    (*info).flags[i] = 1;
                    printf("%s\n", CLIENT_CONFIRM);
                    sendToUDPSocket((*info).serverfd,CLIENT_CONFIRM,new_addr);
                    return 0;
                }
            }
        }
    }

    return 1;
}

int removeFromGroup(struct group_info *info, struct sockaddr_in *client_addr)
{
    int compare = checkAddresses(info, client_addr);

    if(compare < 0)
    {
        sendToUDPSocket((*info).serverfd,ERR_CLIENT_NO_EXIST,client_addr);
        return 1;
    }
    else
    {
        memset((*info).client_addresses[compare], 0, sizeof((*(*info).client_addresses[compare])));
        --(*info).num_clients;
        (*info).flags[compare] = 0;
        sendToUDPSocket((*info).serverfd,CLIENT_REMOVED,client_addr);
    }

    return 0;
}

int broadCast()
{
    return 0;
}


void printGroupInfo(struct group_info *info)
{
    printf("Begin Debug Info\n");

    int errcode = 0;
    char host2[NI_MAXHOST];
    
    for(int i = 0; i < (*info).group_size; ++i)
    {
        if((errcode = getnameinfo(((struct sockaddr *)((*info).client_addresses[i])),sizeof((*(*info).client_addresses[i])),host2,NI_MAXHOST,NULL,0,NI_DGRAM|NI_NUMERICHOST))<0)
        {
            fprintf(stderr, "Error getting host info: %s\n", gai_strerror(errcode));
        }

        printf("Host info at %i: %s\n",i,host2);
    }

    printf("Group name: %s\n",(*info).group_name);
    printf("Max Group Size: %i\n", (*info).group_size);
    printf("Num clients: %i\n", (*info).num_clients);
    
    for(int i = 0; i < (*info).group_size; ++i)
    {
        printf("Flags at %i: %i\n",i, (*info).flags[i]);
    }

    printf("End Debug Info.\n");
}


int initializeSessionInfo(struct group_info *info)
{
    printf("Initializing session info data structure...\n");

    for(int i = 0; i < MAX_GROUP_SIZE; ++i)
    {
        (*info).client_addresses[i] = malloc(sizeof(struct sockaddr_in));
        memset((*info).client_addresses[i],0,sizeof(struct sockaddr_in));
        (*info).flags[i] = 0;
    }
        
    printf("Success.\n");
    return 0;
}

void *listenToServerSocket(void *info_ptr)
{
    printf("Listening for messages on server socket...\n");
    
    struct group_info *info = (struct group_info *) info_ptr;

    const int serverfd = (*info).serverfd;
    const struct sockaddr_in *serveraddr = (*info).server_addr;
    struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
    socklen_t socklen;
    char *climsg;
    
    for (;;)
    {
        memset(client_addr, 0, sizeof((*client_addr)));

        if ((climsg = readFromUDPSocket(serverfd,&socklen,client_addr)) == NULL)
        {

        }
        else
        {
            if (strcmp(climsg, "JOIN") == 0)
            {
                addToGroup(info, client_addr);
            }
            else if (strcmp(climsg, "QUIT") == 0)
            {
                removeFromGroup(info, client_addr);
            }
            else
            {
                fprintf(stderr, "Client sent unrecognized message: %s\n",climsg);
            }
        }

        free(climsg);
    }
}

int messageLoop(struct group_info *info)
{
    int num = 0;
    
    for(;;)
    {
        printf("Please enter the number of messages to broadcast.\n");
        
        char *input = getInput(SMALL_BUFFER_SIZE);
        
        if((num=atoi(input)) < 1)
        {
            fprintf(stderr, "Please enter a number greater than 1.\n");
            free(input);
            continue;
        }

        free(input);

        for(int i = 0; i < num; ++i)
        {
            printf("Please enter a message to send or CLEARALL to unsubscribe all clients:\n");
            input = getInput(DEFAULT_BUFFER_SIZE);

            if(strcmp("CLEARALL",input) == 0)
            {
                free(input);
                break;
            }
            else if (strcmp("debug",input) == 0)
            {
                printGroupInfo(info);
            }

            printf("Sending message :%s to all clients in group.\n",input);
            free(input);
        }

    }

    return 0;
}

int serverProgram(struct group_info *info)
{
    printf("Setup complete. Initializing UDP server program...\n");
   
    initializeSessionInfo(info);

    pthread_t thread_id;

    if(pthread_create(&thread_id,NULL,listenToServerSocket,(void *)info) != 0)
    {
        perror("Error creating thread.\n");
        return 1;
    }
    else
    {
        printf("Thread ID %i is listening for client messages.\n",(int)thread_id);
    }

    messageLoop(info);

    return 0;
}

int main(int argc, char* argv[])
{
    char server_usage[] = "[-p PORT_NUMBER]";

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
                        return usage(argv[0],server_usage);
                    }
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


    struct group_info *info = malloc(sizeof(struct group_info));

    (*info).server_addr = malloc(sizeof(struct sockaddr_in));
    
    if(((*info).serverfd = createUDPServerSocket(port, (*info).server_addr)) < 0)
    {
        return EXIT_FAILURE;
    }

    printf("Please enter a group name:\n");

    (*info).group_name = getInput(SMALL_BUFFER_SIZE);

    char *input;

    while((*info).group_size < 1)
    {
        printf("Please enter maximum number of multicast clients.\n");
        
        input = getInput(SMALL_BUFFER_SIZE);

        if (((*info).group_size = atoi(input)) < 1)
        {
            fprintf(stderr,"Error converting input to int. Please enter a group size >= 1.\n");
        }
        free(input);
    }

    printf("Group Name: %s\n", (*info).group_name);
    printf("Group Size: %i\n", (*info).group_size);

    serverProgram(info);

    return EXIT_SUCCESS;
}
