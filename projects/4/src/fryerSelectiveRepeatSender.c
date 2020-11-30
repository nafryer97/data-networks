#include"fryerSelectiveRepeatSender.h"

int slidingWindowProtocol(int sockfd, const unsigned int ws, struct transfer_stats *stats, struct sockaddr_in *clientaddr, FILE *inpFile, FILE *diags)
{
    char msg[MEDIUM_BUFFER_SIZE] = "";

    int errnum = 0;
    
    unsigned int totPack = 0;
    unsigned int nt = 0;
    unsigned int na = 0;
    unsigned int nf = 0;

    struct frame frBuf[ws];
    struct frame cliFr;

    struct frame eofFr;
    eofFr.kind = ack;
    eofFr.fSize = strlen("end");

    memset(msg,'\0',SMALL_BUFFER_SIZE);

    totPack = ((unsigned int) 
            (((*stats).statbuf.st_size+
                ((*stats).statbuf.st_size % (size_t)MAX_PACK)) 
             / ((size_t)MAX_PACK)));

    snprintf(msg,(MEDIUM_BUFFER_SIZE-1),"Transferring a file size %ju will require %u packets of size %u\n",
            (size_t)(*stats).statbuf.st_size,totPack,MAX_PACK);

    printf("%s\n",msg);

    memset(msg,'\0',MEDIUM_BUFFER_SIZE);

    if(diags!=NULL)
    {
        fprintf(diags,"%s\n",msg);
    }

    if((errnum = notifyReceiverFileSize(sockfd, (size_t)(*stats).statbuf.st_size, clientaddr)) != 0)
    {
        return handleErrorRet(-1,"Abandoning sliding window protocol");
    }

    unsigned short acked[totPack];

    for(int i =0; (i < totPack) && (errnum==0); ++i)
    {
        acked[i] = 0;
    }
    
    for(int i = 0; (i < ws) && (errnum==0) && (feof(inpFile)==0); ++i)
    {
        memset(&frBuf[i],0,sizeof(struct frame));

        frBuf[i].kind=data;

        if((frBuf[i].fSize = getPacketFromFile(frBuf[i].packet,inpFile))<0)
        {
            return handleErrorRet(-1,"Abandoning sliding window protocol");
        }
        
        frBuf[i].seqNo=nf;
        ++nf;
    }

    for(int i = 0; (i < ws) && (errnum==0); ++i)
    {
        snprintf(msg,(SMALL_BUFFER_SIZE-1), "Sending %u...",frBuf[i].seqNo);
        printf("%s\n",msg);
        
        if(diags!=NULL)
        {
            fprintf(diags,"%s\n",msg);
        }
        
        memset(msg,'\0',SMALL_BUFFER_SIZE);
   
        if((errnum=sendFrameUDP(sockfd,&frBuf[i],clientaddr))==0)
        {
            ++(*stats).totPack;
            ++nt;
        }
                
    }
    
    while(errnum==0)
    {
        while(((nt-na)>=ws) && (errnum==0))
        {
            memset(&cliFr,0,sizeof(cliFr));

            if((errnum=readFrameUDP(sockfd,0,NULL,&cliFr))!=0)
            {
                return handleErrorRet(-3,"Error getting frame from receiver");
            }
            if((cliFr.kind == ack) && (cliFr.seqNo < (ws+na)) && (cliFr.seqNo <= totPack))
            {
                acked[cliFr.seqNo] = 1;
                ++(*stats).totAck;

                snprintf(msg,(SMALL_BUFFER_SIZE-1),"Acked %u",cliFr.seqNo);
                greenStdout(msg);
                
                if(diags!=NULL)
                {
                    fprintf(diags,"%s\n",msg);
                }

                memset(msg,'\0',SMALL_BUFFER_SIZE);

                if(cliFr.seqNo == na)
                {
                    ++na;
                }

            }
            else if((cliFr.kind == nack) && (cliFr.seqNo < (ws+na)))
            {
                ++(stats->totNack);
                
                snprintf(msg,(SMALL_BUFFER_SIZE-1),"Nacked %u",cliFr.seqNo);
                redStdout(msg);
                
                if(diags!=NULL)
                {
                    fprintf(diags,"%s\n",msg);
                }

                memset(msg,'\0',SMALL_BUFFER_SIZE);

                if((stats->seqNack[stats->totNack-1] = cliFr.seqNo)==na)
                {
                    break;
                }

            }
            else if((cliFr.kind == ack) && (cliFr.seqNo >= totPack) && (cliFr.fSize==strlen("end")) && (strcmp((char *)cliFr.packet,"end")==0))
            {
                snprintf(msg,(MEDIUM_BUFFER_SIZE-1),"\nClient acked end of file packet.\nTransfer complete.\n");
                greenStdout(msg);
                if(diags!=NULL)
                {
                    fprintf(diags,"%s",msg);
                }

                return 0;
            }
            else if ((cliFr.kind == data) || (cliFr.seqNo > (ws+na)))
            {
                yellowStdout("Receiver sent unexpected data");
                printFrame(&cliFr);
            }
        }

        for(int i = 0; i < ws; ++i)
        {
            if(acked[nf-ws+i] == 1 && (feof(inpFile)==0))
            {
                memset(&frBuf[i],0,sizeof(struct frame));
                frBuf[i].kind=data;

                if((frBuf[i].fSize = getPacketFromFile(frBuf[i].packet,inpFile))<0)
                {
                    return handleErrorRet(-1,"Abandoning sliding window protocol");
                }

                frBuf[i].seqNo=nf;
                ++nf;
            }
        }
        
        for(int i = 0; (i < ws) && (errnum==0); ++i)
        {
            if(acked[frBuf[i].seqNo] == 0)
            {
                snprintf(msg,(SMALL_BUFFER_SIZE-1),"Sending %u...",frBuf[i].seqNo);
                printf("%s\n",msg);

                if(diags!=NULL)
                {
                    fprintf(diags,"%s",msg);
                }

                memset(msg,'\0',SMALL_BUFFER_SIZE);
                                   
                if(((errnum=sendFrameUDP(sockfd,&frBuf[i],clientaddr))==0) && (frBuf[i].seqNo >= nt))
                {
                    ++(stats->totPack);
                    ++nt;
                }
                else if(frBuf[i].seqNo < nt)
                {
                    snprintf(msg, (SMALL_BUFFER_SIZE-1),"Retransmitting %u",frBuf[i].seqNo);
                    yellowStdout(msg);
                    
                    if(diags!=NULL)
                    {
                        fprintf(diags,"%s\n",msg);
                    }

                    memset(msg,'\0',SMALL_BUFFER_SIZE);

                    ++(stats->totRetr);
                }
            }
            else if((na >= totPack) && (feof(inpFile) != 0))
            {
                snprintf(msg,(SMALL_BUFFER_SIZE-1),"Sending end of file notice to receiver");
                
                yellowStdout(msg);

                if(diags!=NULL)
                {
                    fprintf(diags,"%s\n",msg);
                }

                memset(msg,'\0',SMALL_BUFFER_SIZE);

                eofFr.seqNo = na + 1;
                
                strncpy((char *)eofFr.packet,"end",(sizeof(unsigned char)*MAX_PACK)); 
                
                if((errnum = sendFrameUDP(sockfd,&eofFr,clientaddr))==0)
                {
                    ++(*stats).totPack;
                    ++nt;
                    break;
                }
            }
        }
    }
       
    return -2;
}

int transferProgram(int sockfd,int windowSize, struct sockaddr_in *clientaddr,char *fileName,FILE *inpFile,FILE *diags)
{
    struct transfer_stats stats;
    if(statInit(fileName, inpFile, &stats, clientaddr) != 0)
    {
        return handleErrorRet(-1,"Error initializing stats info");
    }

    printf("Using file: %s\nStarting transfer procedures...",fileName);

    if(slidingWindowProtocol(sockfd,windowSize,&stats,clientaddr,inpFile,diags)!=0)
    {
        return handleErrorRet(-1,"Sliding window protocol failed");
    }

    if(diags != NULL)
    {
        printTransferStats(&stats,diags);
    }

    printTransferStats(&stats,NULL);

    return 0;
}


int main(int argc, char* argv[])
{
    int port = 0;
    int windowSize = 0;
    int numUsers = 0;
    int sockfd = 0;
    
    char *fileName = NULL;
    
    struct user_list userList;
    struct sockaddr_in addr;
    struct sockaddr_in clientaddr;
    
    FILE *inpFile = NULL;
    FILE *diags = NULL;
   
    if (argc == 5)
    {
        int opt = 0;
        while((opt = getopt(argc, argv, ":p:w:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    
                    if ((port = parsePortNo(optarg)) == 0)
                    {
                        return usage(argv[0],SENDER_USAGE);
                    }
                    break;
                case 'w':
                    windowSize= atoi(optarg);
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0],SENDER_USAGE);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0],SENDER_USAGE);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0],SENDER_USAGE);
           }
        }
    }
    else
    {
        fprintf(stderr, "Expected 4 arguments. %i were provided.\n", (argc-1));
        return usage(argv[0],SENDER_USAGE);
    }

    setup(port,&sockfd,&numUsers,&addr,&userList,&diags);

    int confirm = -1;

    for(;;)
    {
        if((confirm = getReceiverInfo(sockfd,&clientaddr,&userList,windowSize)) == 0)
        {
            /* Receiver Confirmed that they had received window size */
            if((confirm = getFileInfo(sockfd, &clientaddr, &fileName, &inpFile)) == 0)
            {
                if(transferProgram(sockfd,windowSize,&clientaddr,fileName,inpFile,diags)!=0)
                {
                    break;
                }
                else
                {
                    if(diags != NULL)
                    {
                        fclose(diags);
                    }
                }
            }
            else if (confirm == 1)
            {
                /* receiver sent terminate message */
                break;
            }
            else
            {
                handleFatalError("Exiting....",sockfd);
            }

            
            if(fileName != NULL)
            {
                free(fileName);
            }
            
            if(inpFile != NULL)
            {
                fclose(inpFile);
                inpFile = NULL;
            }
        }
        else if(confirm == 1)
        {
            /* Receiver abandoned the file transfer process */
            continue;
        }
        else if(confirm == 2)
        {
            /* Receiver disconnected */
            break;
        }
        else if(confirm == 3)
        {
            /* Receiver sent an unexpected message */
            continue;
        }
        else
        {
            /* Something went wrong */
            handleFatalError("Exiting...",sockfd);
        }
    }

    close(sockfd);

    greenStdout("\nNo errors were encountered. Goodbye.\n");

    return EXIT_SUCCESS;
}

int notifyReceiverFileSize(int sockfd, size_t fSize, struct sockaddr_in *clientaddr)
{
    printf("Sending file size to receiver...");
    fflush(stdout);
    struct frame fr;
    int errnum = 0;
    memset(&fr,0,sizeof(fr));

    fr.kind = data;
    fr.seqNo = 0;
    fr.fSize = fSize;

    //printFrame(&fr);

    if((errnum=sendFrameUDP(sockfd,&fr,clientaddr))!=0)
    {
        return handleErrorNoRet(errnum,errnum,"Error sending file size to receiver");
    }

    greenStdout("Success");

    memset(&fr,0,sizeof(fr));

    printf("Waiting for ack...");
    fflush(stdout);

    if((errnum=readFrameUDP(sockfd,0,NULL,&fr))==0)
    {
        if(fr.kind==ack && fr.fSize==fSize)
        {
            greenStdout("Received");
            //printFrame(&fr);
        }
        else
        {
            yellowStdout("Receiver sent unexpected data");
            printFrame(&fr);
        }
    }
    else if(errnum == -1)
    {
        return handleErrorRet(-2,"Receiver closed the connection");
    }
    else
    {
       return handleErrorRet(-1,"Error getting receiver file size ack"); 
    }

    return 0;
}

int getPacketFromFile(unsigned char *buf, FILE *inpFile)
{
    size_t ret = fread(buf,sizeof(*buf),MAX_PACK,inpFile);
    if(ret == MAX_PACK)
    {
        /* no errors */
        return ret;
    }
    else if(ret>0)
    {
        /* read some bytes, but not the expected amount */
        if(feof(inpFile)!=0)
        {
            yellowStdout("End of File");
            return ret;
        }
        else if (ferror(inpFile)!=0)
        {
            return handleErrorRet(2, "Error reading from file");
        }
    }
    else
    {
        /* read nothing */
        if(feof(inpFile)!=0)
        {
            return handleErrorRet(-1, "End of file. No bytes read.");
        }
        else if (ferror(inpFile)!=0)
        {
            return handleErrorRet(-2, "Error reading file. No bytes read.");
        }
    }

    return -3;
}

int checkFile(char *fileName, FILE **inpFile)
{
    if(((*inpFile) = fopen(fileName, "r")) == NULL)
    {
        int errnum = errno;
        return handleErrorNoRet(errnum, errnum, "Could not open the file that receiver specified");
    }

    return 0;
}

int getFileInfo(int sockfd, struct sockaddr_in *clientaddr, char **fileName, FILE **inpFile)
{
    int errnum = 0;
    char *errMsg = NULL;
    
    for(;;)
    {
        printf("Waiting for file name...");
        fflush(stdout);
    
        if(((*fileName) = readFromUDPSocket(sockfd,0,NULL)) == NULL)
        {
            return handleErrorRet(-1, "Error");
        }

        greenStdout("Received.");
        
        if(strcmp((*fileName), RECEIVER_TERMINATED) == 0)
        {
            free((*fileName));
            (*fileName) = NULL;
            return 1;
        }

        if((errnum = checkFile((*fileName), inpFile)) != 0)
        {
            errMsg = strerror(errnum);
            handleErrorMsg(errMsg);
            
            if(sendToUDPSocket(sockfd, errMsg, clientaddr) != 0)
            {
                free((*fileName));
                (*fileName) = NULL;
                return handleErrorRet(-1, "Failed to send error message to receiver");
            }
        }
        else
        {
            if(sendToUDPSocket(sockfd, SENDER_ACCEPT, clientaddr) != 0)
            {
                free((*fileName));
                (*fileName) = NULL;
                return handleErrorRet(-1, "Failed to send error message to receiver");
            }

            return 0;
        }
        free((*fileName));
        (*fileName) = NULL;
    }
}

int getReceiverInfo(int sockfd, struct sockaddr_in *clientaddr, struct user_list *userList, int windowSize)
{
    int userInd = -1;
    char accept[SMALL_BUFFER_SIZE] = "";
    char errMsg[SMALL_BUFFER_SIZE] = "";
    char *confirm = NULL;

    if ((userInd = authenticate(sockfd, clientaddr, userList)) < 0)
    {
        return handleErrorRet(-1,"Failed to authenticate");
    }

    snprintf(accept, (SMALL_BUFFER_SIZE-1), "%s:%i", SENDER_ACCEPT,windowSize);

    if(sendToUDPSocket(sockfd, accept, clientaddr) != 0)
    {
        return handleErrorRet(-1,"Failed to send window size");
    }

    printf("Welcome, %s\nWaiting for file transfer confirmation...",(*userList).users[userInd].name);
    fflush(stdout);

    if((confirm = readFromUDPSocket(sockfd,0,NULL)) == NULL)
    {
        return handleErrorRet(-1,"Error waiting for confirmation");
    }

    if(strcmp(confirm, RECEIVER_CONFIRM) == 0)
    {
        greenStdout("Confirmed.");
        free(confirm);
        return 0;
    }
    else if(strcmp(confirm, RECEIVER_N_CONFIRM) == 0)
    {
        yellowStdout("Receiver abandoned file transfer request.\n");
        free(confirm);
        return 1;
    }
    else if(strcmp(confirm, RECEIVER_TERMINATED) == 0)
    {
        yellowStdout("Receiver disconnected.\n");
        free(confirm);
        return 2;
    }
    else
    {
        snprintf(errMsg, (SMALL_BUFFER_SIZE-1), "Unexpected reply: %s\n", confirm);
        free(confirm);
        return handleErrorRet(3,errMsg);
    }
}

int authenticate(int sockfd, struct sockaddr_in *clientaddr, struct user_list *userList)
{
    char errMsg[SMALL_BUFFER_SIZE] = "";
    char *name = NULL;
    char *password = NULL;
    socklen_t socklen = sizeof(struct sockaddr_in);

    for(;;)
    {
        memset(clientaddr, 0, sizeof(struct sockaddr_in));
        printf("Waiting for username...");
        fflush(stdout);
        
        if((name = readFromUDPSocket(sockfd, socklen, clientaddr)) == NULL)
        {
            return handleErrorRet(-1, "Error waiting for username");        
        }

        if(sendToUDPSocket(sockfd, SENDER_CONFIRM, clientaddr) != 0)
        {
            free(name);
            return handleErrorRet(-1, "Error sending confirmation message");
        }

        greenStdout("Received.");
        printf("Waiting for password...");
        fflush(stdout);

        if((password = readFromUDPSocket(sockfd, 0, NULL)) == NULL)
        {
            free(name);
            return handleErrorRet(-1, "Error waiting for password");        
        }

        if(sendToUDPSocket(sockfd, SENDER_CONFIRM, clientaddr) != 0)
        {
            free(name);
            free(password);
            return handleErrorRet(-1, "Error sending confirmation message");
        }

        greenStdout("Received.");
        printf("Verifying...");
        fflush(stdout);

        for(int i = 0; i < NUM_USERS; ++i)
        {
            if((strcmp(name,(*userList).users[i].name) == 0) && (strcmp(password, (*userList).users[i].password) == 0))
            {
                free(name);
                free(password);
                return(i);
            }
        }

        snprintf(errMsg, SMALL_BUFFER_SIZE, "Auth failed: %s %s", name, password);
        redStdout(errMsg);

        free(name);
        free(password);
        
        if(sendToUDPSocket(sockfd, "Auth failed", clientaddr) != 0)
        {
            return handleErrorRet(-1, "Error sending auth message");
        }

    }
}

void setup(int port, int *sockfd, int *numUsers, struct sockaddr_in *addr, struct user_list *userList,FILE **diags)
{
    pthread_t tid;
    int thres = 0;
    int errnum = 0;
    void *thrval = NULL;

    if((thres = pthread_create(&tid, NULL, thr_createUserList, userList))!=0)
    {
        handleFatalErrorNo(thres, "Fatal Error in pthread_create. Exiting...", -1);
    }

    if(((*diags) = fopen("diags_sender","w"))==NULL)
    {
        errnum = errno;
        handleErrorNoMsg(errnum,"Error creating diagnostic file");
    }

    if(((*sockfd)=createUDPServerSocket(port, addr))<0)
    {
        handleFatalError("Exiting...",-1);
    }

    if((thres = pthread_join(tid,&thrval)) != 0)
    {
        handleFatalErrorNo(thres, "Fatal Error in pthread_join. Exiting...", (*sockfd));
    }

    if(thrval != NULL)
    {
        memcpy(numUsers, thrval, sizeof(int));
        free(thrval);
    }
    else
    {
        handleFatalError("User list file thread did not return a value",(*sockfd));
    }

    if((*numUsers) == NUM_USERS)
    {
        printUserList(userList, (*numUsers));
    }
    else if ((*numUsers) < 1)
    {
        handleFatalError("Exiting...",(*sockfd));
    }
    else
    {
        printUserList(userList, (*numUsers));
    }
}

void printUserList(struct user_list *userList, int i)
{
    printf("\n\tBegin User Entries\n\n");

    char namesRead[((7+SMALL_BUFFER_SIZE)*2*NUM_USERS)] = "";
    char entry[((7+SMALL_BUFFER_SIZE)*2)] = "";

    for(int j = 0; j<i;++j)
    {
        snprintf(entry, ((6+SMALL_BUFFER_SIZE)*2), "%i: %s %s\n",j,(*userList).users[j].name,(*userList).users[j].password);
        strncat(namesRead, entry, strlen(entry));

        memset(entry, '\0', sizeof(entry));
    }
    
    printf("%s\n\tEnd\n\n",namesRead);
}

int createUserList(struct user_list *userList)
{
    char name[SMALL_BUFFER_SIZE] = "";
    char password[SMALL_BUFFER_SIZE] = "";
    int numRead = 0;

    FILE *userFile;
    
    printf("Retrieving user info...");

    for (int i=0; i < NUM_USERS; ++i)
    {
        memset((*userList).users[i].name, '\0', (sizeof(char)*SMALL_BUFFER_SIZE));
        memset((*userList).users[i].password, '\0', (sizeof(char)*SMALL_BUFFER_SIZE));
    }

    if ((userFile = fopen(USER_LIST,"r")) == NULL)
    {
        int errnum = errno;
        char errMsg[SMALL_BUFFER_SIZE] = "";
        snprintf(errMsg, (SMALL_BUFFER_SIZE-1), "Error opening user list file: %s", USER_LIST);
        return handleErrorNoRet(errnum, -1, errMsg);
    }

    for(int i=0;i<NUM_USERS;++i)
    {
        if(fscanf(userFile,"%s %s",name,password)!=EOF)
        {
            strncpy((*userList).users[i].name, name, (SMALL_BUFFER_SIZE-1));
            strncpy((*userList).users[i].password, password, (SMALL_BUFFER_SIZE-1));
            memset(name, '\0', (sizeof(char)*SMALL_BUFFER_SIZE));
            memset(password, '\0', (sizeof(char)*SMALL_BUFFER_SIZE));
            ++numRead;
        }
        else if(ferror(userFile) != 0)
        {
            int errnum = errno;
            if (fclose(userFile) != 0)
            {
                handleErrorNoMsg(errnum, "Error reading user list file");
                errnum = errno;
                return handleErrorNoRet(errnum, -1, "Error closing user list file");
            }
            else
            {
                return handleErrorNoRet(errnum, -1, "Error reading user list file");
            }
        }
        else
        {
            break;
        }
    }

    fclose(userFile);
    return numRead;
}

void *thr_createUserList(void *info)
{
    char errMsg[SMALL_BUFFER_SIZE] = "";
    void *retval = malloc(sizeof(int));
    int numUsers = 0;

    memset(retval, 0, sizeof(int));

    struct user_list *userList = (struct user_list *)info;
    
    numUsers = createUserList(userList);

    memcpy(retval, &numUsers, sizeof(int));

    if(numUsers == NUM_USERS)
    {
        greenStdout("Success.");
    }
    else if(numUsers < 1)
    {
        handleErrorMsg("Unable to retrieve users");
    }
    else
    {
        snprintf(errMsg, (SMALL_BUFFER_SIZE-1), "\nFound %i user entries. Expected %i",numUsers,NUM_USERS);
        yellowStdout(errMsg);
    }

    return retval;
}
