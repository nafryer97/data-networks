#include"fryerSelectiveRepeatReceiver.h"

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

int waitForConfirmation(struct sender_info *sender)
{
    char errMsg[MEDIUM_BUFFER_SIZE] = "";
    char *response = NULL;

    yellowStdout("Waiting for confirmation...");

    if((response = readFromUDPSocket((*sender).sockfd,NULL,NULL))==NULL)
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

    yellowStdout("Sending username to server...");
    
    if(sendToUDPSocket((*sender).sockfd, name, &(*sender).serveraddr)!=0)
    {
        return handleErrorRet(-1,"Error sending username to server");
    }

    if((res = waitForConfirmation(sender)) != 0)
    {
        return res;
    }

    yellowStdout("Sending password to server...");

    if(sendToUDPSocket((*sender).sockfd, password, &(*sender).serveraddr)!=0)
    {
        return handleErrorRet(-1,"Error sending password to server");
    }

    if((res = waitForConfirmation(sender)) != 0)
    {
        return res;
    }

    if((response = readFromUDPSocket((*sender).sockfd,NULL,NULL))==NULL)
    {
        return handleErrorRet(-1,"Error waiting for auth message");
    }
    
    if(strncmp(response, SENDER_ACCEPT, strlen(SENDER_ACCEPT)) != 0)
    {
        free(response);
        return handleErrorRet(-3, "Server did not accept credentials");
    }

    //printf("Response is %s\n",response);

    char *tok = response;

    if(strsep(&tok, ":") == NULL)
    {
        free(response);
        return handleErrorRet(-4, "Accept message was not in the expected format");
    }

    if((res = atoi(tok)) < 1)
    {
        free(response);
        return handleErrorRet(-5, "Could not read a window size int from server");
    }

    char msg[SMALL_BUFFER_SIZE] = "";
    snprintf(msg, SMALL_BUFFER_SIZE, "Server accepted credentials. Window size is %i", res);
    greenStdout(msg);

    return res;
}

int receiverProgram(struct sender_info *sender)
{

    return 0;
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

int main(int argc, char* argv[])
{
    char client_usage[] = "[-s SERVER_IP_ADDRESS] [-p PORT_NUMBER] [-i INPUT_FILE] [-o OUTPUT_FILE]";
    int port = 0;
    char* address;
    char *inpFile;
    char *outFile;

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
                        return usage(argv[0],client_usage);
                    }
                    break;
                case 's':
                    address = optarg;
                    break;
                case 'i':
                    inpFile = optarg;
                    break;
                case 'o':
                    outFile = optarg;
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0], client_usage);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0], client_usage);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0], client_usage);
           }
        }
    }
    else
    {
        fprintf(stderr, "%i arguments. Expected 9.\n", argc);
        return usage(argv[0], client_usage);
    }
   
    struct sender_info sender;

    strncpy(sender.addrStr,address,SMALL_BUFFER_SIZE);
    sender.port = port;
    
    if((sender.sockfd = createUDPClientSocket(sender.port,sender.addrStr, &sender.sockfd,&sender.serveraddr)) < 0)
    {
        handleFatalError("Exiting...");
    }

    /*
    pthread_t tid;
    int tres = 0;

    if((tres = pthread_create(&tid,NULL,listenToSender,&sender))==0)
    {
        sleep(1);
    }
    else
    {
        handleFatalErrorNo(tres, "Error creating listener thread");
    }
    */

    struct user_info credentials;

    if(getCredentials(&credentials) < 0)
    {
        handleFatalError("Exiting...");
    }

    printf("Username: %s\n", credentials.name);
    printf("Password: %s\n", credentials.password);

    int windowSize = 0;

    while((windowSize = sendCredentials(credentials.name, credentials.password, &sender)) < 1)
    {
        if(windowSize == -3)
        {
        }
        else if (windowSize == -1)
        {
            handleFatalError("Exiting...");
        }
        else if (windowSize == -2)
        {
            handleFatalError("Exiting...");
        }
        else if (windowSize == -4)
        {
            handleFatalError("Exiting...");
        }
        else if (windowSize == -5)
        {
            handleFatalError("Exiting...");
        }
        
        if(getCredentials(&credentials)<0)
        {
            handleFatalError("Exiting...");
        }
        
        printf("Username: %s\n", credentials.name);
        printf("Password: %s\n", credentials.password);
    }

    /*
    void *tstatus;

    if(pthread_cancel(tid) == 0)
    {
        yellowStdout("Sent cancel signal to listening thread...");

        if ((tres = pthread_join(tid, &tstatus)) == 0)
        {
            printf("Thread exited with value: %i\n",tstatus);
        }
        else
        {
            handleErrorNoMsg(tres,"Failed to join with listening thread");
        }
    }
    else
    {
        if((tres=pthread_join(tid, &tstatus)) == 0)
        {
            printf("Thread exited with value: %s\n",(char *)tstatus);
        }
        else
        {
            handleErrorNoMsg(tres,"Failed to join with listening thread");
        }
    }
    */

    receiverProgram(&sender);

    printf("Goodbye.\n");

    return EXIT_SUCCESS;
}
