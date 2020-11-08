#include"fryerP2Receiver.h"

int serverLoop(int sockfd)
{
    struct sockaddr_in cliaddr;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    
    for(;;)
    {
        memset(&cliaddr, 0, sizeof(struct sockaddr_in));
        
        printf("Waiting for connection...\n");
        
        int senderfd = accept(sockfd, (struct sockaddr*) &cliaddr, &clientlen);

        if (senderfd < 0)
        {
            perror("Error accepting connection.\n");
        }
        else
        {
            printf("Accepted.\n");
        }

        close(senderfd);
        printf("Connection Closed.\n");
    }

    return EXIT_SUCCESS;
}

int usage(char* str)
{
    fprintf(stderr, "Usage: %s -p port_number\n",str);
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

    printf("Port: %i.\n", port);

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
