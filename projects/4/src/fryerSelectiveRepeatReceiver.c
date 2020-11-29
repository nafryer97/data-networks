#include"fryerSelectiveRepeatReceiver.h"

intmax_t  waitForFileSize(struct sender_info *sender)
{
    int errnum = 0;
    struct frame fr;
    char buf[SMALL_BUFFER_SIZE] = "";
   
    memset(&fr,0,sizeof(fr));

    printf("Waiting for file size...");

    if((errnum = readFrameUDP((*sender).sockfd,0,NULL,&fr))==0)
    {
        if(fr.kind==data)
        {
            snprintf(buf, (SMALL_BUFFER_SIZE-1),"Received: %ju",fr.fSize);
            greenStdout(buf);
            printFrame(&fr);
        }
        else
        {
            return handleErrorRet(-1,"Expected file size from sender");
            printFrame(&fr);
        }
    }
    else if(errnum == -1)
    {
        return handleErrorRet(-2,"Sender closed the connection");
    }
        
    printf("Sending ack...");
    fflush(stdout);

    fr.kind = ack;

    if((errnum=sendFrameUDP((*sender).sockfd,&fr,&(*sender).serveraddr))!=0)
    {
        return handleErrorNoRet(errnum,errnum,"Error sending ack");
    } 
    else
    {
        greenStdout("Success");
    }

    return fr.fSize;
}

int writePacket(FILE *outFile, const unsigned char *pack)
{
   size_t ret = fwrite(pack,sizeof(*pack),MAX_PACK,outFile);
   if(ret == MAX_PACK)
   {    
       return 0;
   }
   else if(ret>0)
   {
       /* wrote some stuff, but not everything */
       return -1;
   }
   else
   {
       /* wrote nothing */
       return -2;
   }
}

int fileTransfer(FILE *outFile, struct sender_info *sender, unsigned int wr)
{
    int errnum = 0;
    unsigned int totPack = 0;
    unsigned int nr = 0;
    unsigned int ns = 1;
    intmax_t fileSize = 0;
    struct frame frBuf[wr];
    struct frame fr;

    if((fileSize=waitForFileSize(sender))<=0)
    {
        handleErrorRet(-1,"Abandoning file transfer");
    }
    
    totPack = (unsigned int)((fileSize+(fileSize % ((intmax_t)MAX_PACK)))/((intmax_t)MAX_PACK));

    printf("Transferring a file size %ju will require %u packets of size %u\n",
            fileSize,totPack,MAX_PACK);
   
    memset(&fr,0,sizeof(fr));
    while((errnum=readFrameUDP((*sender).sockfd,0,NULL,&fr))==0)
    {
        if((fr.seqNo == nr) && (fr.kind==data))
        {
            //frame arrived in order
            writePacket(outFile, fr.packet);
            fr.kind=ack;
            ++nr;
        }
        else if((fr.kind == data) && (fr.seqNo > nr) && (fr.seqNo < (wr+nr)) && (1 < wr))
        {
            // frame arrived out of order but within window, buffer it
            memcpy(&frBuf[fr.seqNo-nr],&fr,sizeof(fr));
            fr.kind=ack;
        }
        else if((fr.kind == nack) && (fr.seqNo <= nr))
        {
            // sender lost one of our acks
            fr.kind=ack;
        }
        else
        {
            // frame arrived outside of window, discard it
            fr.kind=nack;
        }

        //send ack or nack
        if((errnum=sendFrameUDP((*sender).sockfd,&fr,&(*sender).serveraddr))!=0)
        {
            handleErrorNoRet(errnum,errnum,"Abandoning file transfer");
        }

        //check if acked frame is the highest seqno we've received so far
        if(fr.kind==ack && fr.seqNo >= ns)
        {
            ns=fr.seqNo+1;
        }

        //check to see if the buffer contains the next packet in the sequence
        for(int i = 0;(i < wr) && ((nr+i) < ns) && (frBuf[i].seqNo!=0) && (1 < wr); ++i)
        {
            if(frBuf[i].seqNo == nr)
            {
                writePacket(outFile, frBuf[i].packet);
                memset(&frBuf[i],0,sizeof(struct frame));
                ++nr;
            }
        }

        if(nr==totPack)
        {
            greenStdout("Complete");
            break;
        }

        memset(&fr,0,sizeof(fr));
    }
    
    return 0;
}

int main(int argc, char* argv[])
{
    int port = 0;
    int windowSize = 0;
    
    char msg[SMALL_BUFFER_SIZE] = "";
    char* address;
    char *inpFile;
    char *outName;

    FILE *outFile = NULL;

    struct sender_info sender;
    struct user_info credentials;
    
    if (argc == 9)
    {
        int opt = 0;
        while((opt = getopt(argc, argv, ":s:p:i:o:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    port = parsePortNo(optarg);
                    if (!port)
                    {
                        return usage(argv[0],RECEIVER_USAGE);
                    }
                    break;
                case 's':
                    address = optarg;
                    break;
                case 'i':
                    inpFile = optarg;
                    break;
                case 'o':
                    outName = optarg;
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0], RECEIVER_USAGE);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0], RECEIVER_USAGE);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0], RECEIVER_USAGE);
           }
        }
    }
    else
    {
        fprintf(stderr, "%i arguments. Expected 8.\n", (argc-1));
        return usage(argv[0], RECEIVER_USAGE);
    }

    setup(port, address,outName,&outFile,&credentials, &sender, &windowSize);
 
    snprintf(msg, (SMALL_BUFFER_SIZE-1), "Authentication Successful. Window size is %i", windowSize);
    greenStdout(msg);

    int res = confirmTransfer(&sender);

    if(res == 0)
    {
        /* sent confirmation */
        if((res = sendFileInfo(inpFile, &sender)) == 0)
        {
            /* server accepted file name */
            if((res = fileTransfer(outFile, &sender,windowSize)) != 0)
            {
                handleFatalError("Exiting...",sender.sockfd);
            }
            

        }
        else if(res == 1)
        {
            /* receiver sent terminate message */

        }
        else 
        {
            /* an error occurred */
            handleFatalError("Exiting...",sender.sockfd);
        }
        
    }
    else if (res == 1)
    {
        /* sent cancel */
    }
    else if (res == 2)
    {
        /* sent terminate */
    }
    else if (res < 0)
    {
        /* error */
        handleFatalError("Exiting...",sender.sockfd);
    }
    else
    {
        /* error? */
        handleFatalError("Exiting...",sender.sockfd);
    }

    
    close(sender.sockfd);

    greenStdout("\nNo errors were encountered. Goodbye.\n");

    return EXIT_SUCCESS;
}

void setup(int port, char *address, char *outName, FILE **outFile, struct user_info *credentials, struct sender_info *sender, int *windowSize)
{
    strncpy((*sender).addrStr,address,SMALL_BUFFER_SIZE);
    (*sender).port = port;
    
    if(((*sender).sockfd = createUDPClientSocket((*sender).port,(*sender).addrStr, &(*sender).sockfd,&(*sender).serveraddr)) < 0)
    {
        handleFatalError("Exiting...", -1);
    }

    if(((*outFile) = fopen(outName,"w"))==NULL)
    {
        handleFatalErrorNo(errno, "Error creating output file",(*sender).sockfd);
    }

    if(getCredentials(credentials) < 0)
    {
        handleFatalError("Exiting...", (*sender).sockfd);
    }

    while(((*windowSize) = sendCredentials((*credentials).name, (*credentials).password, sender)) < 1)
    {
        if((*windowSize) == -3)
        {
            /* Authentication failed. try again. */
        }
        else if ((*windowSize) == -1)
        {
            /* socket error */
            handleFatalError("Exiting...", (*sender).sockfd);
        }
        else if ((*windowSize) == -2)
        {
            /* confirmation was in wrong format */
            handleFatalError("Exiting...", (*sender).sockfd);
        }
        else if ((*windowSize) == -4)
        {
            /* accept message was in wrong format */
            handleFatalError("Exiting...", (*sender).sockfd);
        }
        else if ((*windowSize) == -5)
        {
            /* error converting accept message to window size */
            handleFatalError("Exiting...", (*sender).sockfd);
        }
        
        if(getCredentials(credentials)<0)
        {
            handleFatalError("Exiting...", (*sender).sockfd);
        }
    }
}

int confirmTransfer(struct sender_info *sender)
{
    char *inp = NULL;
    
    printf("Start file transfer? [y/n/c]: ");
    fflush(stdout);

    if((inp = getInput(SMALL_BUFFER_SIZE)) == NULL)
    {
        return handleErrorRet(-1, "Error reading input");
    }
    else
    {
        while((strcmp(inp, "y") != 0) && (strcmp(inp, "n") != 0) && (strcmp(inp, "c") != 0))
        {
            printf("Unknown option: %s\nPlease enter \'y\',\'n\', or \'c\' after the prompt:\n%s", inp, RECEIVER_PROMPT);
            fflush(stdout);
            free(inp);

            if ((inp = getInput(SMALL_BUFFER_SIZE)) == NULL)
            {
                return handleErrorRet(-1,"Error reading input");
            }
        }
    }

    if(strcmp(inp, "y") == 0)
    {
        free(inp);
        if(sendToUDPSocket((*sender).sockfd, RECEIVER_CONFIRM, &(*sender).serveraddr) != 0) 
        {
            return handleErrorRet(-1,"Error sending confirmation");
        }
       
        return 0;
    }
    else if(strcmp(inp,"n") == 0)
    {
        free(inp);
        if(sendToUDPSocket((*sender).sockfd, RECEIVER_N_CONFIRM, &(*sender).serveraddr) != 0) 
        {
            return handleErrorRet(-1,"Error sending confirmation");
        }

        return 1;
    }
    else if(strcmp(inp,"c") == 0)
    {
        free(inp);
        if(sendToUDPSocket((*sender).sockfd, RECEIVER_TERMINATED, &(*sender).serveraddr) != 0) 
        {
            return handleErrorRet(-1,"Error sending confirmation");
        }

        return 2;
    }

    return 3;
}

int sendFileInfo(char *inpFile, struct sender_info *sender)
{
    char *response = NULL;
    short flag = 0;

    printf("Sending file name %s...",inpFile);

    for(;;)
    {
        if(sendToUDPSocket((*sender).sockfd, inpFile, &(*sender).serveraddr) != 0)
        {
            return handleErrorRet(-1,"Error sending file name");
        }

        if((response = readFromUDPSocket((*sender).sockfd,0,NULL))==NULL)
        {
            return handleErrorRet(-1,"Error waiting for confirmation receipt");
        }

        if(strcmp(response, SENDER_ACCEPT) == 0)
        {
            return 0;
        }
        else
        {
            fprintf(stderr, "Response from server:\n");
            handleErrorMsg(response);
        }

        if(flag == 1)
        {
            free(inpFile);
        }

        printf("Please enter a new file name, or \"cancel\" to abort:\n");

        if((inpFile = getInput(DEFAULT_BUFFER_SIZE)) == NULL)
        {
            return handleErrorRet(-1,"Error getting file name");
        }

        flag = 1;

        if(strcmp(inpFile, "cancel") == 0)
        {
            free(inpFile);
            
            if(sendToUDPSocket((*sender).sockfd, RECEIVER_TERMINATED, &(*sender).serveraddr) != 0)
            {
                return handleErrorRet(-1,"Error sending message");
            }

            return 1;
        }

        printf("Sending file name %s...",inpFile);
    }
}

int waitForConfirmation(struct sender_info *sender)
{
    char errMsg[MEDIUM_BUFFER_SIZE] = "";
    char *response = NULL;

    //yellowStdout("Waiting for confirmation...");

    if((response = readFromUDPSocket((*sender).sockfd,0,NULL))==NULL)
    {
        return handleErrorRet(-1,"Error waiting for confirmation receipt");
    }
    
    if(strcmp(response, SENDER_CONFIRM)==0)
    {
        greenStdout("Confirmed");
        free(response);
        return 0;
    }
    else
    {
        snprintf(errMsg, MEDIUM_BUFFER_SIZE, "Unexpected response: %s",response);
        free(response);
        return handleErrorRet(-2, errMsg);
    }
}

int sendCredentials(const char *name, const char *password, struct sender_info *sender)
{
    int res = 0;
    char *response = NULL;

    printf("Sending username to server...");
    fflush(stdout);
    
    if(sendToUDPSocket((*sender).sockfd, name, &(*sender).serveraddr)!=0)
    {
        return handleErrorRet(-1,"Error sending username to server");
    }

    if((res = waitForConfirmation(sender)) != 0)
    {
        return res;
    }

    printf("Sending password to server...");
    fflush(stdout);

    if(sendToUDPSocket((*sender).sockfd, password, &(*sender).serveraddr)!=0)
    {
        return handleErrorRet(-1,"Error sending password to server");
    }

    if((res = waitForConfirmation(sender)) != 0)
    {
        return res;
    }

    if((response = readFromUDPSocket((*sender).sockfd,0,NULL))==NULL)
    {
        return handleErrorRet(-1,"Error waiting for auth message");
    }
    
    if(strncmp(response, SENDER_ACCEPT, strlen(SENDER_ACCEPT)) != 0)
    {
        free(response);
        return handleErrorRet(-3, "Authentication failed.");
    }

    //printf("Response is %s\n",response);

    char *tok = response;

    if(strsep(&tok, ":") == NULL)
    {
        free(response);
        return handleErrorRet(-4, "Message was not in the expected format");
    }

    if((res = atoi(tok)) < 1)
    {
        free(response);
        return handleErrorRet(-5, "Error interpreting sender's window size.");
    }


    return res;
}

int getCredentials(struct user_info *credentials)
{
    printf("Please enter your username:");

    char *input;
    
    if ((input = getInput(SMALL_BUFFER_SIZE)) == NULL)
    {
        return handleErrorRet(-1,"");
    }

    strncpy((*credentials).name, input, SMALL_BUFFER_SIZE);

    free(input);

    printf("Please enter your password:");

    if ((input = getInput(SMALL_BUFFER_SIZE)) == NULL)
    {
        return handleErrorRet(-1,"");
    }
    
    strncpy((*credentials).password, input, SMALL_BUFFER_SIZE);

    free(input);

    return 0;
}

/*
void *listenToSender(void *info)
{
    char *retval = malloc(sizeof(char) * SMALL_BUFFER_SIZE);
    memset(retval, '\0', (sizeof(char) * SMALL_BUFFER_SIZE));

    struct sender_info *sender = (struct sender_info *)info;
    
    printf("Separate thread is listening for messages from sender...\n");
    
    char *senderMsg = NULL;
    socklen_t socklen = sizeof(struct sockaddr_in);
    
    for(;;)
    {
        if((senderMsg = readFromUDPSocket((*sender).sockfd,&socklen,NULL))!=NULL)
        {
            printf("Message from sender:\n%s\n",senderMsg);
            free(senderMsg);
            senderMsg = NULL;
            strncpy(retval, THREAD_SOCKFDREAD_SUCC, SMALL_BUFFER_SIZE);
        }
        else
        {
            strncpy(retval, THREAD_SOCKFDREAD_ERR, SMALL_BUFFER_SIZE);
            break;
        }
        
        pthread_testcancel();
    }

    return retval;
}
*/
