#include"fryerSelectiveRepeatReceiver.h"

void *listenToSender(void *info)
{
    struct sender_info *sender = (struct sender_info *)info;

    if(((*sender).sockfd = createUDPClientSocket((*sender).port,(*sender).addrStr, &(*sender).sockfd,&(*sender).serveraddr)) < 0)
    {
        return NULL;
    }
    else
    {
        char *senderMsg = NULL;
        socklen_t socklen = sizeof(struct sockaddr_in);
        for(;;)
        {
            if((senderMsg = readFromUDPSocket((*sender).sockfd,&socklen,NULL))!=NULL)
            {
                printf("Message from sender:\n%s\n",senderMsg);
                free(senderMsg);
                senderMsg = NULL;
            }
            else
            {
                break;
            }
        }
    }

    return NULL;
}

int receiverProgram(struct user_info *credentials)
{
    return 0;
}

int getCredentials(struct user_info *credentials)
{
    printf("Please enter your username:");

    char *input = getInput(SMALL_BUFFER_SIZE);

    strncpy((*credentials).name, input, SMALL_BUFFER_SIZE);

    free(input);

    printf("Please enter your password:");

    input = getInput(SMALL_BUFFER_SIZE);
    
    strncpy((*credentials).password, input, SMALL_BUFFER_SIZE);

    free(input);
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
        return usage(argv[0], client_usage);
    }
   
    struct sender_info sender;

    strncpy(sender.addrStr,address,SMALL_BUFFER_SIZE);
    sender.port = port;

    pthread_t thread_id;
    memset(&thread_id, 0, sizeof(thread_id));

    if(pthread_create(&thread_id,NULL,listenToSender,&sender)==0)
    {
        printf("Separate thread is listening for messages from sender...\n");
        sleep(1);
    }
    else
    {
        fprintf(stderr, "Error creating new thread for listening.\n");
        return EXIT_FAILURE;
    }

    struct user_info credentials;
    memset(&credentials, 0, sizeof(struct user_info));

    getCredentials(&credentials);

    printf("Username: %s\n", credentials.name);
    printf("Password: %s\n", credentials.password);

    if(pthread_cancel(thread_id) == 0)
    {
        fprintf(stderr,"Sent cancel signal to listening thread...\n");

        if (pthread_join(thread_id, NULL) == 0)
        {
            fprintf(stderr,"Joined with listening thread.\n");
        }
        else
        {
            fprintf(stderr,"Failed to join with listening thread.\n");
        }
    }

    printf("Goodbye.\n");

    return EXIT_SUCCESS;
}
