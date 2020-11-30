#include"fryerSelectiveRepeatReceiver.h"

size_t  waitForFileSize(struct sender_info *sender)
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
            //printFrame(&fr);
        }
        else
        {
            return handleErrorRet(-1,"Expected file size from sender");
            //printFrame(&fr);
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

int writePacket(FILE *outFile, struct frame *fr)
{
    /*
    yellowStdout("Writing packet");
    printFrame(fr);
    printf("%ju\n",fr->fSize);
    printf("%lu\n",sizeof(*fr->packet));
    */
    size_t ret = fwrite(fr->packet,sizeof(*fr->packet),fr->fSize,outFile);
    if(ret == fr->fSize)
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

int fileTransfer(FILE *outFile, struct sender_info *sender, unsigned int wr, struct transfer_stats *stats)
{
    int errnum = 0;
    short dropFlag = 0;
    unsigned int totPack = 0;
    unsigned int nr = 0;
    unsigned int ns = 1;
    size_t fileSize = 0;
    struct frame frBuf[wr];
    struct frame fr;

    if((fileSize=waitForFileSize(sender))<=0)
    {
        handleErrorRet(-1,"Abandoning file transfer");
    }
    
    totPack = ((unsigned int)
                    ((fileSize+
                      (fileSize % ((size_t)MAX_PACK))) /
                        ((size_t)MAX_PACK)));

    printf("Transferring a file size %ju will require %u packets of size %u\n",
            fileSize,totPack,MAX_PACK);
   
    memset(&fr,0,sizeof(fr));
    while((errnum=readFrameUDP(sender->sockfd,0,NULL,&fr))==0)
    {
        if((nr % 10 == 0) && (dropFlag == 0) && (fr.kind == data))
        {
            fr.kind=nack;
            dropFlag=1;
        }
        else if((fr.seqNo == nr) && (fr.kind==data))
        {
            //frame arrived in order
            writePacket(outFile, &fr);
            fr.kind=ack;
            ++nr;
            dropFlag = 0;
            //yellowStdout("Packet written");
        }
        else if((fr.seqNo >= totPack) && (fr.kind == ack) && (fr.fSize == strlen("end")) && (strcmp((char *)fr.packet,"end")==0))
        {
            /* sender says this is the end of the file */
            greenStdout("Sender completed transfer");
            sendFrameUDP(sender->sockfd,&fr,&sender->serveraddr);
            return 0;
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
        for(int i = 0;(i < wr) && ((nr+i) <= ns) && (frBuf[i].seqNo >= nr) && (wr > 1); ++i)
        {
            if(frBuf[i].seqNo == nr)
            {
                writePacket(outFile, &frBuf[i]);
                memset(&frBuf[i],0,sizeof(struct frame));
                ++nr;
            }
        }
        
        memset(&fr,0,sizeof(fr));
    }
    
    return 0;
}

int main(int argc, char* argv[])
{
    int windowSize = 0;
    
    char msg[SMALL_BUFFER_SIZE] = "";
    char inpFile[SMALL_BUFFER_SIZE] = "";
    char outName[SMALL_BUFFER_SIZE] = "";

    FILE *outFile = NULL;

    struct sender_info sender;
    struct user_info credentials;
    struct transfer_stats trStats;

    memset(&sender,0,sizeof(sender));

    if (argc == 9)
    {
        int opt = 0;
        while((opt = getopt(argc, argv, ":s:p:i:o:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    sender.port = parsePortNo(optarg);
                    if (sender.port <= 0)
                    {
                        return usage(argv[0],RECEIVER_USAGE);
                    }
                    break;
                case 's':
                    strncpy(sender.addrStr,optarg,(SMALL_BUFFER_SIZE-1));
                    break;
                case 'i':
                    strncpy(inpFile,optarg,(SMALL_BUFFER_SIZE-1));
                    break;
                case 'o':
                    strncpy(outName,optarg,(SMALL_BUFFER_SIZE-1));
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

    setup(outName,&outFile,&credentials,&trStats,&sender,&windowSize);
 
    snprintf(msg, (SMALL_BUFFER_SIZE-1), "Authentication Successful. Window size is %i", windowSize);
    greenStdout(msg);

    int res = confirmTransfer(&sender);

    if(res == 0)
    {
        /* sent confirmation */
        if((res = sendFileInfo(inpFile, &sender)) == 0)
        {
            /* server accepted file name */
            if((res = fileTransfer(outFile,&sender,windowSize,&trStats)) != 0)
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

void *thr_setup(void *thr_setup_info)
{
    int errnum = 0;
    struct thr_setup *thr_info = (struct thr_setup *)thr_setup_info;
    short *ret = malloc(sizeof(short));
    *ret = 0;
    
    if((thr_info->sender->sockfd = createUDPClientSocket(thr_info->sender->port,thr_info->sender->addrStr, &thr_info->sender->sockfd,&thr_info->sender->serveraddr)) < 0)
    {
        handleFatalError("Exiting...", -1);
    }

    if(((*thr_info->outFile) = fopen(thr_info->outName,"w"))==NULL)
    {
        errnum = errno;
        handleFatalErrorNo(errnum, "Error creating output file",thr_info->sender->sockfd);
    }
    
    if(statInit(thr_info->outName,*thr_info->outFile,thr_info->trStats,&thr_info->sender->serveraddr)!=0)
    {
        handleFatalError("Error initializing stats struct",thr_info->sender->sockfd);
    }
    
    return ret;
}

void setup(char *outName, FILE **outFile, struct user_info *credentials,struct transfer_stats *trStats, struct sender_info *sender, int *windowSize)
{
    struct thr_setup thr_setup_info;
    int errnum = 0;
    short *thrval = NULL;
    pthread_t thrid;

    thr_setup_info.sender = sender;
    thr_setup_info.trStats = trStats;
    thr_setup_info.outFile = outFile;
    thr_setup_info.outName = outName;

    if(pthread_create(&thrid,NULL,thr_setup,&thr_setup_info)!=0)
    {
        handleFatalError("Error creating setup thread",-1);
    }
    
    if(getCredentials(credentials) < 0)
    {
        handleFatalError("Exiting...",-1);
    }

    if((errnum = pthread_join(thrid,(void *)&thrval))!=0)
    {
        handleFatalErrorNo(errnum,"Error joining with setup thread",-1);
    }

    if(thrval == NULL || (*thrval)!=0)
    {
        handleFatalError("Setup thread returned invalid data",-1);
    }
    else
    {
        free(thrval);
    }

    while(((*windowSize) = sendCredentials(credentials->name,credentials->password, sender)) < 1)
    {
        switch (*windowSize)
        {
            case -3: /* Authentication failed. try again. */
                break;
            case -1: /* socket error */
            case -2: /* confirmation was in wrong format */
            case -4: /* accept message was in wrong format */
            case -5: /* error converting accept message to window size */
            default:
            handleFatalError("Exiting...",sender->sockfd);
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
        if(sendToUDPSocket(sender->sockfd,RECEIVER_CONFIRM, &sender->serveraddr) != 0) 
        {
            return handleErrorRet(-1,"Error sending confirmation");
        }
       
        return 0;
    }
    else if(strcmp(inp,"n") == 0)
    {
        free(inp);
        if(sendToUDPSocket(sender->sockfd, RECEIVER_N_CONFIRM, &sender->serveraddr) != 0) 
        {
            return handleErrorRet(-1,"Error sending confirmation");
        }

        return 1;
    }
    else if(strcmp(inp,"c") == 0)
    {
        free(inp);
        if(sendToUDPSocket(sender->sockfd, RECEIVER_TERMINATED, &sender->serveraddr) != 0) 
        {
            return handleErrorRet(-1,"Error sending confirmation");
        }

        return 2;
    }

    return 3;
}

int sendFileInfo(char *inpFile, struct sender_info *sender)
{
    char rbuf[MEDIUM_BUFFER_SIZE] = "";
    char *rptr = NULL;
    char *iptr = NULL;
    int errnum = 0;

    printf("Sending file name %s...",inpFile);

    while(errnum==0)
    {
        if((errnum = sendToUDPSocket(sender->sockfd, inpFile, &sender->serveraddr)) != 0)
        {
            return handleErrorRet(-1,"Error sending file name");
        }

        if((rptr = readFromUDPSocket(sender->sockfd,0,NULL))==NULL)
        {
            return handleErrorRet(-1,"Error waiting for confirmation receipt");
        }
        else
        {
            strncpy(rbuf,rptr,MEDIUM_BUFFER_SIZE-1);
            free(rptr);
            rptr = NULL;
        }

        if(strcmp(rbuf, SENDER_ACCEPT) == 0)
        {
            return 0;
        }
        else
        {
            fprintf(stderr, "Response from server:\n");
            handleErrorMsg(rbuf);
            memset(rbuf,'\0',MEDIUM_BUFFER_SIZE);
        }

        printf("Please enter a new file name, or \"cancel\" to abort:\n");

        if((iptr = getInput(DEFAULT_BUFFER_SIZE)) == NULL)
        {
            return handleErrorRet(-1,"Error getting file name");
        }
        else
        {
            strncpy(inpFile,iptr,(SMALL_BUFFER_SIZE-1));
            free(iptr);
            iptr = NULL;
        }

        if(strcmp(inpFile, "cancel") == 0)
        {
            sendToUDPSocket(sender->sockfd, RECEIVER_TERMINATED, &sender->serveraddr);

            return 1;
        }

        printf("Sending file name %s...",inpFile);
    }
    return -1;
}

int waitForConfirmation(struct sender_info *sender)
{
    char errMsg[MEDIUM_BUFFER_SIZE] = "";
    char *response = NULL;

    //yellowStdout("Waiting for confirmation...");

    if((response = readFromUDPSocket(sender->sockfd,0,NULL))==NULL)
    {
        return handleErrorRet(-1,"Error waiting for confirmation receipt");
    }
    else
    {
        strncpy(errMsg,response,(MEDIUM_BUFFER_SIZE-1));
        free(response);
        response = NULL;
    }
    
    if(strcmp(errMsg, SENDER_CONFIRM)==0)
    {
        greenStdout("Confirmed");
        return 0;
    }
    else
    {
        return handleErrorRet(-2, errMsg);
    }
}

int sendCredentials(const char *name, const char *password, struct sender_info *sender)
{
    int res = 0;
    char *response = NULL;

    printf("Sending username to server...");
    fflush(stdout);
    
    if(sendToUDPSocket(sender->sockfd, name, &sender->serveraddr)!=0)
    {
        return handleErrorRet(-1,"Error sending username to server");
    }

    if((res = waitForConfirmation(sender)) != 0)
    {
        return res;
    }

    printf("Sending password to server...");
    fflush(stdout);

    if(sendToUDPSocket(sender->sockfd, password, &sender->serveraddr)!=0)
    {
        return handleErrorRet(-1,"Error sending password to server");
    }

    if((res = waitForConfirmation(sender)) != 0)
    {
        return res;
    }

    if((response = readFromUDPSocket(sender->sockfd,0,NULL))==NULL)
    {
        return handleErrorRet(-1,"Error waiting for auth message");
    }
    
    if(strncmp(response, SENDER_ACCEPT, strlen(SENDER_ACCEPT)) != 0)
    {
        free(response);
        return handleErrorRet(-3, "Authentication failed.");
    }

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

    strncpy(credentials->name, input, SMALL_BUFFER_SIZE);

    free(input);

    printf("Please enter your password:");

    if ((input = getInput(SMALL_BUFFER_SIZE)) == NULL)
    {
        return handleErrorRet(-1,"");
    }
    
    strncpy(credentials->password, input, SMALL_BUFFER_SIZE);

    free(input);

    return 0;
}
