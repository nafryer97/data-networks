#include"fryerSelectiveRepeatCommon.h"

char* readFromUDPSocket(int sockfd, socklen_t *sock_len, struct sockaddr_in *sockaddr)
{
    int n = 0;

    char* sbuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(sbuf, '\0', DEFAULT_BUFFER_SIZE);
    
    if((n = recvfrom(sockfd,sbuf,(DEFAULT_BUFFER_SIZE-1),0,(struct sockaddr *)sockaddr,sock_len)) < 0)
    {
        perror("Error receiving data from socket.\n");
            
        free(sbuf);
        return NULL;
    }
            
    return sbuf;
}

int sendToUDPSocket(int sockfd, char* str, struct sockaddr_in *dest)
{
    int errsv = 0;
    
    if (sendto(sockfd,str,(strlen(str)+1),MSG_CONFIRM,(struct sockaddr *) dest,sizeof(struct sockaddr_in))<0)
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
        if(*nl == '\0')
        {
            break;
        }
        else if(*nl == '\n')
        {
            *nl = '\0';
        }

        ++nl;
    }
}

char* getInput(int buffer_size)
{
    char* inputBuf = malloc(sizeof(char) * buffer_size); 
    memset(inputBuf, '\0', (sizeof(char) * buffer_size));

    char* argument = NULL;
        
    while(argument == NULL)
    {
        argument = fgets(inputBuf, buffer_size, stdin);
        
        if (*(inputBuf + (strlen(inputBuf) - 1)) != '\n')
        {
            printf("Input too long. Please try again.\n");
            argument = NULL;
            memset(inputBuf, '\0', (sizeof(char) * buffer_size));
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

int createUDPClientSocket(int port, const char* address, int *sockfd, struct sockaddr_in *cliaddress)
{
    printf("Attempting to open a UDP client socket for address %s and port %i...\n", address,port);

    memset(cliaddress,0,sizeof(struct sockaddr_in));
 
    (*cliaddress).sin_family = AF_INET;
    (*cliaddress).sin_port=htons(port);
    if(inet_aton(address,&(*cliaddress).sin_addr)==0)
    {
        fprintf(stderr, "Error creating byte address from supplied address: %s\n",address);
    }

    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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

int createUDPServerSocket(int port, struct sockaddr_in *serveraddr)
{
    printf("Attempting to open a UDP server socket for port %i...\n",port);

    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    
    if (sockfd<0)
    {
        perror("Error in socket.\n");
        return sockfd;
    }
    else
    {
        printf("Socket opened.\n");
    }

    memset(serveraddr,0,sizeof(struct sockaddr_in));

    (*serveraddr).sin_family = AF_INET;
    (*serveraddr).sin_port=htons(port);
    (*serveraddr).sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*) serveraddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Error in binding.\n");
        close(sockfd);
        return -1;
    }
    else
    {
        printf("Bound successfully.\n");
        return sockfd;
    }
}

int usage(char *arg1, char *arg2)
{
    fprintf(stderr, "Usage: %s %s\n",arg1,arg2);
    return EXIT_SUCCESS;
}
