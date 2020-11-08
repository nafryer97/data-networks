#include"project-common.h"

char* readFromSocket(int sockfd)
{
    int nread = 0;

    char* sbuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(sbuf, '\0', DEFAULT_BUFFER_SIZE);
    
    do {
        nread = recv(sockfd,sbuf,(DEFAULT_BUFFER_SIZE-1),0);
            
        if (nread < 0)
        {
            perror("Error receiving data from socket.\n");
            free(sbuf);
            return NULL;
        }

    } while(nread == 0);
   
    return sbuf;
}

int sendToSocket(int sockfd, char* str)
{
    int errsv = 0;
    
    if (send(sockfd,str,(strlen(str)+1),0)<0)
    {
        perror("Error sending message to socket.");
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
        if(*nl == '\n')
        {
            *nl = '\0';
        }

        ++nl;
    }
}

char* getInput()
{
    char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
    memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));

    char* argument = NULL;
        
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

int parsePortNo(char* arg)
{
    int port = 0;
    if ((port = atoi(arg)) != 0)
    {
        if (port < DEFAULT_MIN_PORT || port > DEFAULT_MAX_PORT)
        {
            printf("Please use a port between %i and %i\n",DEFAULT_MIN_PORT, DEFAULT_MAX_PORT);
            port = 0;
        }
    }
    else
    {
        printf("Error converting port argument to integer.\n");
    }

    return port;
}

int createClientSocket(int port, const char* address, int *sockfd, struct sockaddr_in *cliaddress)
{
    printf("Attempting to open a client socket for address %s and port %i...\n", address,port);

    memset(cliaddress,0,sizeof(struct sockaddr_in));
 
    (*cliaddress).sin_family = AF_INET;
    (*cliaddress).sin_port = htons(port);

    (*cliaddress).sin_addr.s_addr = inet_addr(address);
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (*sockfd < 0)
    {
        perror("Socket Error.\n");
    }
    else
    {
        printf("Socket Opened.\n");
    }

    return *sockfd;
}

int createServerSocket(int port, struct sockaddr_in *serveraddr)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    if (sockfd<0)
    {
        perror("Error in socket.\n");
        return -1;
    }
    else
    {
        printf("Socket Opened.\n");
    }   

    memset(serveraddr, 0, sizeof(struct sockaddr_in));

    (*serveraddr).sin_port=htons(port);
    (*serveraddr).sin_addr.s_addr=htonl(0);

    if (bind(sockfd, (struct sockaddr*) serveraddr, sizeof(struct sockaddr_in))<0)
    {
        perror("Error in binding.\n");
        close(sockfd);
        return -1;
    }
    else
    {
        printf("Bound Successfully.\n");
        return sockfd;
    }
}
