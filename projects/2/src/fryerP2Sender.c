#include"fryerP2Sender.h"

int readFromServer(int sockfd)
{
    int nread = 0;
    int errsv = 0;

    char* sbuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(sbuf, '\0', DEFAULT_BUFFER_SIZE);
    
    do {
        nread = recv(sockfd,sbuf,(DEFAULT_BUFFER_SIZE-1),0);
            
        if (nread < 0)
        {
            perror("Error receiving response from server.");
            errsv = errno;
            free(sbuf);
            return errsv;
        }

    } while(nread == 0);
    
    if (nread > 0)
    {
        printf("From Server: %s\n", sbuf);
    }
   
    free(sbuf);
    return 0;
}

int sendToServer(int sockfd, char* str)
{
    int errsv = 0;
    
    if (send(sockfd,str,(strlen(str)+1),0)<0)
    {
        perror("Error sending input to server.");
        errsv = errno;
        return errsv;
    }

    return 0;
}

void removeNewLine(char* str)
{
    char* nl = str;
    int len = strlen(str);
    for(int i = 0; i <= len; ++i)
    {
        if(*nl == '\n' && i == len)
        {
            *nl = '\0';
        }
        else if(*nl == '\n' && i < len)
        {
            *nl = ' ';
        }
    }
}

char* getInput()
{
    char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
    memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));

    char* argument = NULL;
        
    printf("Please enter the server's ip address.\n");
        
    while(argument == NULL)
    {
        argument = fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
        
        if (*(inputBuf + (strlen(inputBuf) - 1)) != '\n')
        {
            printf("Input too long. Please try again.\n");
            argument = NULL;
            memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));
        }
    }
    
    removeNewLine(inputBuf);
    argument = strndup(inputBuf, (strlen(inputBuf)+1));
    
    free(inputBuf);
    
    return argument;
}

int setUp(char* address, int port)
{
    int sockfd;
    struct sockaddr_in sockaddress;

    memset(&sockaddress,0,sizeof(struct sockaddr_in));
 
    sockaddress.sin_family = AF_INET;
    sockaddress.sin_port = htons(port);

    sockaddress.sin_addr.s_addr = inet_addr(address);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("Socket Error.\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Socket Opened.\n");
    }

    for(;;)
    {
        if (connect(sockfd, (struct sockaddr *) &sockaddress, sizeof(sockaddress)) < 0)
        {
             perror("Connection Error.\n");
             sleep(2);
             continue;
        }
        else
        {
            printf("Connection Successful.\n");
        }

        return EXIT_SUCCESS;
    }
}

int usage()
{
    printf("Usage: -s <server address> -p <port number> \n");
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    int port = 0;
    char* address;
    
    if (argc > 1 && argc < 6)
    {
        char* args[2];
        int opt = 0;
        while((opt = getopt(argc, argv, ":s:p:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    args[0] = optarg;
                    if ((port = atoi(args[0])) != 0)
                    {
                        if (port < DEFAULT_MIN_PORT || port > DEFAULT_MAX_PORT)
                        {
                            printf("Please use a port between %i and %i\n",DEFAULT_MIN_PORT, DEFAULT_MAX_PORT);
                            return usage();
                        }
                    }
                    else
                    {
                        printf("Error converting port argument to integer.\n");
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

    return EXIT_SUCCESS;
}
