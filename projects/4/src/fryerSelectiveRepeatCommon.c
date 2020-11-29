#include"fryerSelectiveRepeatCommon.h"

int readFrameUDP(int sockfd, socklen_t sock_len, struct sockaddr_in *addr, struct frame *fr)
{
    size_t n = 0;
    int errnum = 0;

    if((n=recvfrom(sockfd,fr,sizeof(struct frame),0,(struct sockaddr *)addr,&sock_len)) < 0)
    {
        errnum =  errno;
        return handleErrorNoRet(errnum,errnum,"Error receiving frame from socket");
    }
    else if (n==0)
    {
        return handleErrorRet(-1, "Connection was closed");
    }

    return 0;
}

char* readFromUDPSocket(int sockfd, socklen_t sock_len, struct sockaddr_in *sockaddr)
{
    int n = 0;

    char* sbuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(sbuf, '\0', DEFAULT_BUFFER_SIZE);
    
    if((n = recvfrom(sockfd,sbuf,(DEFAULT_BUFFER_SIZE-1),0,(struct sockaddr *)sockaddr,&sock_len)) < 0)
    {
        int errnum = errno;
        handleErrorNoMsg(errnum,"Error receiving data from socket");
            
        free(sbuf);
        return NULL;
    }
            
    return sbuf;
}

int sendFrameUDP(int sockfd, const struct frame *fr, const struct sockaddr_in *dest)
{
    int errnum = 0;

    if(sendto(sockfd,fr,sizeof((*fr)),MSG_CONFIRM,(struct sockaddr *)dest,sizeof((*dest)))<0)
    {
        errnum=errno;
        return handleErrorNoRet(errnum,errnum,"Error sending frame to socket.");
    }

    return 0;
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

void printFrame(const struct frame *fr)
{
    char buf[DEFAULT_BUFFER_SIZE] = "";
    const char *kind = NULL;
    struct frame frCpy;

    memcpy(&frCpy,fr,sizeof(frCpy));
    
    if(fr->kind == data)
    {
        kind  = "Kind = data";
    }
    else if(fr->kind == ack)
    {
        kind = "Kind = ack";
    }
    else
    {
       kind = "Kind = nack";
    }

    frCpy.packet[MAX_PACK-1] = '\0';

    snprintf(buf,(DEFAULT_BUFFER_SIZE-1),
            "\nFrame Info:\n%s\nSequence Number: %u\nFile Size: %ju\nPacket Info: \n%s\n\n",
            kind,frCpy.seqNo,frCpy.fSize,frCpy.packet);

    printf("%s",buf);
}

void printTransferStats(struct transfer_stats *stats)
{
    char statsBuf[(DEFAULT_BUFFER_SIZE*2)] = "";
    char seqNackBuf[DEFAULT_BUFFER_SIZE] = "";
    char nackEntry[14] = "";

    printf("\n\tPrinting Statistics\n");

    if ((*stats).totNack > 0)
    {
        for(int i = 0; i < MAX_NACK_SEQ; ++i)
        {
            if (strlen(seqNackBuf) > (DEFAULT_BUFFER_SIZE-6))
            {
                /* buffer overrun */
                break;
            }
            else if (i == ((*stats).totNack-1))
            {
                /* last entry */
                snprintf(nackEntry,13,"%5u",(*stats).seqNack[i]);
                strncat(seqNackBuf,nackEntry,13);
                break;
            } 
            else
            {
                snprintf(nackEntry,13,"%4u, ",(*stats).seqNack[i]);
                strncat(seqNackBuf,nackEntry,13);
                memset(nackEntry,'\0',(sizeof(char)*14));
            }
        }
    }

    snprintf(statsBuf,
            ((DEFAULT_BUFFER_SIZE*2)-1),
            "Receiver address: %s Port: %-9hu\nFile Name: %s File Size: %ju bytes\nFile Creation Date & Time: %sNumber of Data Packets Transmitted: %u\nNumber of Packets Re-transmitted: %u\nNumber of Acknowledgements Received: %u\nNumber of Negative Acknowledgements Received %u\nSequence numbers of negative acknowledgements: %s",
            inet_ntoa((*stats).recvaddr.sin_addr),
            ntohs((*stats).recvaddr.sin_port),
            (*stats).fileName,(intmax_t)(*stats).statbuf.st_size,
            ctime(&(*stats).statbuf.st_mtime),
            (*stats).totPack,(*stats).totRetr,(*stats).totAck,(*stats).totNack,seqNackBuf);

    cyanStdout(statsBuf);

    printf("\n\tComplete\n");
}
