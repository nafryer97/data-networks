#include"client.h"

int main(int argc, char** argv)
{ 
    int clisoc;
    int re;
    
    char* ipAddress;

    char recbuff[BUFF_SIZE];
    
    struct sockaddr_in cliaddr;
    bzero(&cliaddr,sizeof(cliaddr));

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(CLI_PORT);

    /*
     * We need the ip address of the server 
     */
    if (argc > 1)
    {
        //argv[0] is just the name of the executable
        ipAddress = argv[1];
    }
    else
    {
        //no command-line arguments were provided, ask for ip address
        char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
        memset(inputBuf, '\0', DEFAULT_BUFFER_SIZE);

        char* argument = NULL;
        
        printf("Please provide the server's IP address.\n");
        
        while(argument == NULL)
        {
            argument = fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
            if (inputBuf[strlen(inputBuf)-1] != '\n')
            {
                printf("Input too long. Please try again.\n");
                argument = NULL;
            }
        }
        
        ipAddress = argument;
    }

    cliaddr.sin_addr.s_addr = inet_addr(ipAddress);
    clisoc = socket(AF_INET, SOCK_STREAM, 0);

    if (clisoc < 0)
    {
        perror("Socket Error.\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Socket Opened.\n");
    }

    if (connect(clisoc, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
    {
        perror("Connection Error.\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Connection Successful.\n");
    }

    if ((re = read(clisoc, recbuff, sizeof(recbuff))) < 0)
    {
        perror("Read Error.\n");
        return EXIT_FAILURE;
    }

    printf("The current data and time is %s\n",recbuff);
    close(clisoc);

    return EXIT_SUCCESS;
}
