#include"fryerSelectiveRepeatCommon.h"

void redStdout(const char *msg)
{
    printf("\033[0;31m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void greenStdout(const char *msg)
{
    printf("\033[0;32m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void yellowStdout(const char *msg)
{
    printf("\033[0;33m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void blueStdout(const char *msg)
{
    printf("\033[0;34m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void magentaStdout(const char *msg)
{
    printf("\033[0;35m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void cyanStdout(const char *msg)
{
    printf("\033[0;36m");
    printf("%s\n",msg);
    printf("\033[0m");
}

char* readFromUDPSocket(int sockfd, socklen_t *sock_len, struct sockaddr_in *sockaddr)
{
    int n = 0;

    char* sbuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(sbuf, '\0', DEFAULT_BUFFER_SIZE);
    
    if((n = recvfrom(sockfd,sbuf,(DEFAULT_BUFFER_SIZE-1),0,(struct sockaddr *)sockaddr,sock_len)) < 0)
    {
        int errnum = errno;
        handleErrorNoMsg(errnum,"Error receiving data from socket");
            
        free(sbuf);
        return NULL;
    }
            
    return sbuf;
}

int sendToUDPSocket(int sockfd, const char* str, struct sockaddr_in *dest)
{
    int errnum = 0;
    
    if (sendto(sockfd,str,(strlen(str)+1),MSG_CONFIRM,(struct sockaddr *) dest,sizeof(struct sockaddr_in))<0)
    {
        errnum = errno;
        return handleErrorNoRet(errnum, errnum,"Error sending message to socket");
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
        if((argument = fgets(inputBuf, buffer_size, stdin))==NULL)
        {
            int errnum = errno;
            handleErrorNoMsg(errnum, "Failed to get input from stdin");
            free(inputBuf);
            return NULL;
        }
        
        if (*(inputBuf + (strlen(inputBuf) - 1)) != '\n')
        {
            yellowStdout("Input too long. Please try again.");
            argument = NULL;
            memset(inputBuf, '\0', (sizeof(char) * buffer_size));
        }
    }
    
    removeNewLine(argument);
    
    return argument;
}

int parsePortNo(char* arg)
{
    int port = 0;
    char errMsg[SMALL_BUFFER_SIZE] = "";
    if ((port = atoi(arg)) != 0)
    {
        if (port < DEFAULT_MIN_PORT || port > DEFAULT_MAX_PORT)
        {
            snprintf(errMsg, SMALL_BUFFER_SIZE,"Please use a port between %i and %i",DEFAULT_MIN_PORT, DEFAULT_MAX_PORT);
            handleErrorMsg(errMsg);
            port = 0;
        }
    }
    else
    {
        snprintf(errMsg, SMALL_BUFFER_SIZE,"Error converting port argument \"%s\" to integer", arg);
        handleErrorMsg(errMsg);
    }

    return port;
}

int createUDPClientSocket(int port, const char* address, int *sockfd, struct sockaddr_in *cliaddress)
{
    char errMsg[MEDIUM_BUFFER_SIZE] = "";
    
    snprintf(errMsg, MEDIUM_BUFFER_SIZE,"Attempting to open a UDP client socket for address %s and port %i...", address,port);

    yellowStdout(errMsg);
    
    memset(cliaddress,0,sizeof(struct sockaddr_in));
 
    (*cliaddress).sin_family = AF_INET;
    (*cliaddress).sin_port=htons(port);
    if(inet_aton(address,&(*cliaddress).sin_addr)==0)
    {
        snprintf(errMsg, MEDIUM_BUFFER_SIZE,"Error creating byte address from supplied address: %s",address);
        handleErrorMsg(errMsg);
    }

    if(((*sockfd) = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        int errnum = errno;
        handleErrorNoMsg(errnum, "Socket Error");
    }
    else
    {
        greenStdout("Socket Opened");
    }

    return *sockfd;
}

int createUDPServerSocket(int port, struct sockaddr_in *serveraddr)
{
    char msg[SMALL_BUFFER_SIZE] = "";
    snprintf(msg, SMALL_BUFFER_SIZE,"Attempting to open a UDP server socket for port %i...",port);
    yellowStdout(msg);

    int sockfd = -1; 
    
    if ((sockfd = socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        int errnum = errno;
        return handleErrorNoRet(errnum, sockfd, "Error in Socket");
    }
    else
    {
        greenStdout("Socket opened.");
    }

    memset(serveraddr,0,sizeof(struct sockaddr_in));

    (*serveraddr).sin_family = AF_INET;
    (*serveraddr).sin_port=htons(port);
    (*serveraddr).sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*) serveraddr, sizeof(struct sockaddr_in)) < 0)
    {
        close(sockfd);
        int errnum = errno;
        return handleErrorNoRet(errnum, -1,"Error in binding");
    }
    else
    {
        greenStdout("Bound successfully");
        return sockfd;
    }
}

int usage(char *arg1, char *arg2)
{
    fprintf(stderr, "Usage: %s %s\n",arg1,arg2);
    return EXIT_SUCCESS;
}

void handleFatalErrorNo(int en, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s: %s\n",msg,strerror(en)); 
    exit(EXIT_FAILURE);
}

int handleErrorNoRet(int en, int retval, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s: %s\n", msg, strerror(en));
    fprintf(stderr, "\033[0m");
    return retval;
}

void handleErrorNoMsg(int en, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s: %s\n", msg, strerror(en));
    fprintf(stderr, "\033[0m");
}

void handleFatalError(const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s\n",msg); 
    exit(EXIT_FAILURE);
}

int handleErrorRet(int retval, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s\n", msg);
    fprintf(stderr, "\033[0m");
    return retval;
}

void handleErrorMsg(const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s\n", msg);
    fprintf(stderr, "\033[0m");
}
