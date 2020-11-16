#include"fryerP3Client.h"

int clientProgram(int sockfd, struct sockaddr_in serveraddr, char *group_name)
{
    printf("Please enter a message to send to server.\n\"JOIN\" to join %s\n\"QUIT\"to leave %s\n\"EXIT\" to end program.\n",group_name,group_name);

    char *input = getInput(SMALL_BUFFER_SIZE);
    char *serverMsg = NULL;
    socklen_t server_len = sizeof(serveraddr);

    while(strcmp(input, "EXIT") != 0)
    {
        if(strcmp(input, "JOIN") == 0)
        {
            sendToUDPSocket(sockfd,input,&serveraddr);
            printf("Waiting for reply...\n");
            for(;;)
            {
                if((serverMsg = readFromUDPSocket(sockfd,&server_len,&serveraddr)) != NULL)
                {
                    printf("Message from server: %s\n", serverMsg);
                    free(serverMsg);
                    break;
                }
            }
        }
        else if(strcmp(input, "QUIT") == 0)
        {
            sendToUDPSocket(sockfd,input,&serveraddr);
            printf("Waiting for reply...\n");
            for(;;)
            {
                if((serverMsg = readFromUDPSocket(sockfd,&server_len,&serveraddr)) != NULL)
                {
                    printf("Message from server: %s\n", serverMsg);
                    free(serverMsg);
                    break;
                }
            }
        }
        else if(strcmp(input, "EXIT") == 0)
        {
        }
        else
        {
            printf("Unknown command: %s\n",input);
        }

        free(input);
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
        return EXIT_SUCCESS;
    }
    else
    {
        close(sockfd);
        return EXIT_FAILURE;
    }
}
