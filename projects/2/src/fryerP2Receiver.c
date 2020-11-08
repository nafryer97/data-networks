#include"fryerP2Receiver.h"

int hamming(int sockfd, char *arg1, char *arg2)
{
    printf("Calculating hamming distance...\n");
    
    int i = 0, count = 0;

    char *ptr1 = arg1;
    char *ptr2 = arg2;
    
    while(((*ptr1) != '\0') && ((*ptr2) != '\0'))
    {
        if ((*ptr1) != (*ptr2))
        {
            ++count;
        }
        printf("%c %c %i |",(*ptr1),(*ptr2),count);
        ++ptr1;
        ++ptr2;
    }

    char result[DEFAULT_BUFFER_SIZE] = "";

    snprintf(result,DEFAULT_BUFFER_SIZE,"Hamming Distance for %s and %s: %i",arg1,arg2,count);

    if(sendToSocket(sockfd, result) < 0)
    {
        return -1;
    }

    return 1;
}

int getMessages(int sockfd)
{
    int status = -1;
    char *msg1 = NULL;
    char *msg2 = NULL;

    printf("Waiting for message 1 from relay...\n");

    if((msg1 = readFromSocket(sockfd)) == NULL)
    {
        return status;
    }

    if(strcmp(msg1, "CLOSE") == 0)
    {
        printf("Received close message.\n");
        status = 0;
        free(msg1);
        return status;
    }
    
    printf("Message 1: %s\n",msg1);

    printf("Waiting for message 2 from relay...\n");
    
    if((msg2 = readFromSocket(sockfd)) == NULL)
    {
        free(msg1);
        return status;
    }
    if(strcmp(msg1, "CLOSE") == 0)
    {
        printf("Received close message.\n");
        status = 0;
        free(msg1);
        free(msg2);
        return status;
    }

    printf("Message 2: %s\n",msg2);
    
    status = hamming(sockfd, msg1, msg2);

    free(msg1);
    free(msg2);

    return status;
}

int serverLoop(int sockfd)
{
    int status = 1; 

    struct sockaddr_in cliaddr;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    
    memset(&cliaddr, 0, sizeof(struct sockaddr_in));
        
    printf("Waiting for connection...\n");
        
    int relayfd = accept(sockfd, (struct sockaddr*) &cliaddr, &clientlen);

    if (relayfd < 0)
    {
        perror("Error accepting connection.\n");
    }
    else
    {
        printf("Accepted. Connected to relay server.\n");
        while(status == 1)
        {
            status = getMessages(relayfd);
        }
        close(relayfd);
        printf("Connection Closed.\n");
    }
    
    close(sockfd);

    printf("Exiting with status %i",status);

    return EXIT_SUCCESS;
}

int usage(char* str)
{
    fprintf(stderr, "Usage: %s [-p port number]\n",str);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    int port = 0;
    if (argc > 2 && argc < 4)
    {
        char* arg;
        int opt = 0;
        while((opt = getopt(argc, argv, ":p:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    port = parsePortNo(optarg);
                    if (port==0)
                    {
                        return usage(argv[0]);
                    }
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0]);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0]);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0]);
           }
        }
    }
    else
    {
        return usage(argv[0]);
    }

    struct sockaddr_in serveraddr;
    int sockfd = 0;

    if((sockfd = createServerSocket(port, &serveraddr)) < 0)
    {
        return EXIT_FAILURE;
    }

    if(listen(sockfd,50)<0)
    {
        perror("Error setting socket to listen.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }
    
    return serverLoop(sockfd);
}
