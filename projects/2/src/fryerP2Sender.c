#include"fryerP2Sender.h"

int senderProgram(int sockfd)
{
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
            return EXIT_SUCCESS;
        }
    
        if((response = readFromSocket(sockfd)) != NULL)
        {
            if (atoi(response) == 0)
            {
                printf("Welcome!\n");
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
        free(input);
    }

    return EXIT_SUCCESS;
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
