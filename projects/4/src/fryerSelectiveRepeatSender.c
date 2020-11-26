#include"fryerSelectiveRepeatSender.h"

int statInit(char *fileName, FILE *inpFile, struct transfer_stats *stats, struct sockaddr_in *clientaddr)
{
    int fd = -1;
    int errnum = -1;
   
    printf("Initializing transfer statistics structure...");
    fflush(stdout);

    memset(stats, 0, sizeof((*stats)));
    memcpy(&(*stats).recvaddr,clientaddr,sizeof((*stats).recvaddr)); 
    
    if((fd = fileno(inpFile)) == -1)
    {
        errnum = errno;
        return handleErrorNoRet(errnum, -1, "Error retrieving file descriptor");
    }

    if(fstat(fd,&(*stats).statbuf) != 0)
    {
        errnum = errno;
        return handleErrorNoRet(errnum, -1, "Error retrieving file stats");
    }

    strncpy((*stats).fileName, fileName, SMALL_BUFFER_SIZE);

    greenStdout("Success.");

    return 0;
}

void printTransferStats(struct transfer_stats *stats)
{
    char statsBuf[(DEFAULT_BUFFER_SIZE*2)] = "";
    char seqNackBuf[DEFAULT_BUFFER_SIZE] = "";
    char nackEntry[14] = "";
   
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
            "Receiver address: %s Port: %-9hu\nFile Name: %s File Size: %ju bytes\nFile Creation Date & Time: %s\nNumber of Data Packets Transmitted: %u\nNumber of Packets Re-transmitted: %u\nNumber of Acknowledgements Received: %u\nNumber of Negative Acknowledgements Received %u\n Sequence numbers of negative acknowledgements: %s",
            inet_ntoa((*stats).recvaddr.sin_addr),
            ntohs((*stats).recvaddr.sin_port),
            (*stats).fileName,(intmax_t)(*stats).statbuf.st_size,
            ctime(&(*stats).statbuf.st_mtime),
            (*stats).totPack,(*stats).totRetr,(*stats).totAck,(*stats).totNack,seqNackBuf);

    cyanStdout(statsBuf);
}

int transferProgram(int sockfd,struct sockaddr_in *clientaddr,char *fileName,FILE *inpFile)
{
    struct frame fr;
    struct transfer_stats stats;
    int errnum = 0;

    printf("Starting transfer procedures...");

    memset(&fr, 0, sizeof(fr));

    if(statInit(fileName, inpFile, &stats, clientaddr) != 0)
    {
        return handleErrorRet(-1,"Error initializing stats info");
    }

    printTransferStats(&stats);

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

    setup(port,&sockfd,&numUsers,&addr,&userList);

    int confirm = -1;

    for(;;)
    {
        if((confirm = getReceiverInfo(sockfd,&clientaddr,&userList,windowSize)) == 0)
        {
            /* Receiver Confirmed that they had received window size */
            if((confirm = getFileInfo(sockfd, &clientaddr, &fileName, &inpFile)) == 0)
            {
                printf("Using file: %s\n", fileName);
                transferProgram(sockfd,&clientaddr,fileName,inpFile);
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
    
        if(((*fileName) = readFromUDPSocket(sockfd,NULL,NULL)) == NULL)
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

    if((confirm = readFromUDPSocket(sockfd,NULL,NULL)) == NULL)
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
        
        if((name = readFromUDPSocket(sockfd, &socklen, clientaddr)) == NULL)
        {
            return handleErrorRet(-1, "Error waiting for username");        
        }

        if(sendToUDPSocket(sockfd, SENDER_CONFIRM, clientaddr) != 0)
        {
            free(name);
            return handleErrorRet(-1, "Error sending confirmation message");
        }

        printf("Received.\nWaiting for password...");
        fflush(stdout);

        if((password = readFromUDPSocket(sockfd, &socklen, clientaddr)) == NULL)
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

        printf("Received.\nVerifying...");
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

void setup(int port, int *sockfd, int *numUsers, struct sockaddr_in *addr, struct user_list *userList)
{
    pthread_t tid;
    int thres = 0;
    void *thrval = NULL;

    if((thres = pthread_create(&tid, NULL, thr_createUserList, userList))!=0)
    {
        handleFatalErrorNo(thres, "Fatal Error in pthread_create. Exiting...", -1);
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
    char namesRead[((7+SMALL_BUFFER_SIZE)*2*NUM_USERS)] = "";
    char entry[((7+SMALL_BUFFER_SIZE)*2)] = "";

    for(int j = 0; j<i;++j)
    {
        snprintf(entry, ((6+SMALL_BUFFER_SIZE)*2), "%i: %s %s\n",j,(*userList).users[j].name,(*userList).users[j].password);
        strncat(namesRead, entry, strlen(entry));

        memset(entry, '\0', sizeof(entry));
    }
    
    cyanStdout(namesRead);
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
