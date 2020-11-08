#include"fryerP2Sender.h"

int sendMessages(int serverfd)
{
    int status = -1;
    char *response = NULL;
    if((response = readFromSocket(serverfd)) != NULL)
    {
        printf("Response: %s\n", response);
        if (atoi(response) == 0)
        {
            printf("Relay connected to receiver.\n");
        }
        else
        {
            printf("Relay could not connect to receiver.\n");
            free(response);
            return status;
        }
    }

    free(response);
    response = NULL;

    printf("Please enter a message to send to the receiver, or \"CLOSE\" to close the connection.\n");

    char *msg = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
    char *input = NULL;
    size_t msgLen = 0;
    int numSent = 1;
    input = getInput();

    while(strcmp(input, "CLOSE") != 0)
    {
        memset(msg, '\0', (sizeof(char)*DEFAULT_BUFFER_SIZE));
        msgLen = strlen(input);
        
        if(snprintf(msg, DEFAULT_BUFFER_SIZE, "%i %s", ((int)msgLen), input)<0)
        {
            fprintf(stderr, "Error formatting message string.\n");
            break;
        }

        if(sendToSocket(serverfd, msg) != 0)
        {
            fprintf(stderr, "Error sending message to relay server.\n");
            break;
        }

        if((response = readFromSocket(serverfd)) != NULL)
        {
            printf("Response: %s\n", response);
            free(response);
            response = NULL;
        }
        else
        {
            fprintf(stderr, "Error getting response from relay.\n");
            break;
        }

        if((numSent % 2) == 0)
        {
            printf("Expecting hamming distance from receiver....\n");
            if ((response = readFromSocket(serverfd)) != NULL)
            {
                printf("Response: %s\n", response);
                free(response);
                response = NULL;
            }
            else
            {
                fprintf(stderr, "Error getting response from relay.\n");
                break;
            }
        }

        free(input);
        input = NULL;

        ++numSent;
        printf("Please enter a message to send to the receiver, or \"CLOSE\" to close the connection.\n");
        input = getInput();
    }

    free(msg);

    if(input != NULL)
    {
        if (strcmp(input, "CLOSE") == 0)
        {
            status = 0;
        }

        free(input);
    }

    return status;
}

int contactReceiver(int sockfd)
{
    int status = -1;

    char *input = NULL;
    char *response = NULL;

    printf("Enter \'x\' or \'q\' to quit.\n");
    printf("Please enter a server name:\n");

    input = getInput();

    while((strcmp("x", input) != 0) && (strcmp("q",input) != 0))
    {
        sendToSocket(sockfd, input);
        free(input);
        input = NULL;

        if((response = readFromSocket(sockfd)) != NULL)
        {
            printf("Response: %s\n", response);
            free(response);
            response = NULL;
        }
        
        printf("Please enter a port number:\n");
        
        input = getInput();

        if((strcmp("x", input) != 0) && (strcmp("q",input) != 0))
        {
            sendToSocket(sockfd, input);
            free(input);
            input = NULL;
        }
        else
        {
            status = 0;
            break;
        }
    
        if((response = readFromSocket(sockfd)) != NULL)
        {
            if (atoi(response) == 0)
            {
                printf("Relay accepted server info. Waiting for connection...\n");
                free(response);
                response = NULL;
                status = 1;
                break;
            }
            else
            {
                printf("Incorrect server/port number. Please try again.\n");
            }

            free(response);
            response = NULL;
        }

        printf("Enter \'x\' or \'q\' to quit.\n");
        printf("Please enter a server name:\n");

        input = getInput();
    }

    if(input != NULL)
    {
        if((strcmp("x", input) == 0) || (strcmp("q",input) == 0))
        {
            status = 0;
        }

        free(input);
    }
    
    return status;
}

int authenticate(int sockfd)
{
    int status = -1;
    char* input = NULL;
    char* response = NULL;

    printf("Enter \'x\' or \'q\' to quit.\n");
    printf("Please enter a username:\n");

    input = getInput();

    while((strcmp("x", input) != 0) && (strcmp("q",input) != 0))
    {
        sendToSocket(sockfd, input);
        free(input);
        input = NULL;

        if((response = readFromSocket(sockfd)) != NULL)
        {
            printf("Response: %s\n", response);
            free(response);
            response = NULL;
        }
        
        printf("Please enter a password:\n");
        
        input = getInput();

        if((strcmp("x", input) != 0) && (strcmp("q",input) != 0))
        {
            sendToSocket(sockfd, input);
            free(input);
            input = NULL;
        }
        else
        {
            free(input);
            status = 0;
            return status;
        }
    
        if((response = readFromSocket(sockfd)) != NULL)
        {
            if (atoi(response) == 0)
            {
                printf("Welcome!\n");
                free(response);
                response = NULL;
                status = 1;
                return status;
            }
            else
            {
                printf("Incorrect username/password. Please try again.\n");
            }

            free(response);
            response = NULL;
        }

        printf("Enter \'x\' or \'q\' to quit.\n");
        printf("Please enter a username:\n");

        input = getInput();
    }

    if(input != NULL)
    {
        if((strcmp("x", input) == 0) || (strcmp("q",input) == 0))
        {
            status = 0;
        }
        free(input);
    }

    return status;
}

int senderProgram(int sockfd)
{
    int status = 0;

    for(;;)
    {
        if((status = authenticate(sockfd)) <  1)
        {
            printf("Exiting with status: %i\n", status);
            break;
        }
        if((status = contactReceiver(sockfd)) < 1)
        {
            printf("Exiting with status: %i\n", status);
            break;
        }
        if((status = sendMessages(sockfd)) < 0)
        {
            printf("Exiting with status: %i\n", status);
            break;
        }
    }
    printf("Goodbye.\n");

    return status;
}

int usage()
{
    printf("Usage: ./server -s <server address> -p <port number> \n");
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
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
                        return usage();
                    }
                    break;
                case 's':
                    args[1] = optarg;
                    address = args[1];
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage();
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage();
                default:
                    printf("Something unexpected occurred.\n");
                    return usage();
           }
        }
    }
    else
    {
        return usage();
    }

    printf("Server: %s.\n", address);
    printf("Port: %i.\n", port);

    struct sockaddr_in cliaddress;
    int sockfd;

    if ((sockfd = createClientSocket(port, address, &sockfd, &cliaddress)) < 0)
    {
        return EXIT_FAILURE;
    }
    
    printf("Sender attempting to contact relay server.\n");
    
    for(int attempts = 0;attempts < 5;++attempts)
    {
        if (connect(sockfd, (struct sockaddr*) &cliaddress, sizeof(cliaddress)) < 0)
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
            senderProgram(sockfd);
            close(sockfd);
            return EXIT_SUCCESS;
        }
    }
    
    return EXIT_FAILURE;
}
