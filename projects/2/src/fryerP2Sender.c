#include"fryerP2Sender.h"

int clientLoop(int sockfd)
{
    printf("Sender attempting to contact relay server.\n");
    for(int attempts = 0;attempts < 5;++attempts)
    {
        if (connect(sockfd, (struct sockaddr *) &sockaddress, sizeof(sockaddress)) < 0)
        {
             perror("Attempt %i of 5: connection error.\n", (attempts+1));
             sleep(2);
        }
        else
        {
            printf("Attempt %i: connection successful.\n", (attempts+1));
            close(sockfd);
            return EXIT_SUCCESS;
        }
    }
    
    return EXIT_FAILURE;
}

int usage()
{
    printf("Usage: ./server -s <server address> -p <port number> \n");
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    int port = 0;
    char* address;
    
    if (argc > 1 && argc < 6)
    {
        char* args[2];
        int opt = 0;
        while((opt = getopt(argc, argv, ":s:p:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    args[0] = optarg;
                    port = parsePortNo(args[0]);
                    if (!port)
                    {
                        return usage();
                    }
                    break;
                case 's':
                    args[1] = optarg;
                    address = args[1];
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage();
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage();
                default:
                    printf("Something unexpected occurred.\n");
                    return usage();
           }
        }
    }
    else
    {
        return usage();
    }

    printf("Server: %s.\n", address);
    printf("Port: %i.\n", port);

    int relayfd = 0;
    if ((relayfd = setUpClientSocket(address,port)) < 0)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return clientLoop(relayfd);        
    }
}
