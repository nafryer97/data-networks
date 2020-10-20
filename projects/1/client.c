#include"client.h"

int main(int argc, char** argv)
{ 
    int clisoc;
    int re;
    int cli_Port;

    char* ipAddress;

    char recbuff[DEFAULT_BUFFER_SIZE];
    
    struct sockaddr_in cliaddr;
    bzero(&cliaddr,sizeof(cliaddr));

    /*
     * We need the ip address and port  of the server 
     */
    if (argc > 2)
    {
        //argv[0] is just the name of the executable
        ipAddress = argv[1];
        cli_Port = atoi(argv[2]);
    }
    else
    {
        //
        //no command-line arguments were provided, ask for ip address
        //
        //https://www.geeksforgeeks.org/why-to-use-fgets-over-scanf-in-c/
        //
        //
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
        
        strcpy(ipAddress, argument);

        printf("Please enter the port.\n");

        memset(inputBuf, '\0', DEFAULT_BUFFER_SIZE);
        argument = NULL;

        while(argument == NULL)
        {
            argument = fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
            if (inputBuf[strlen(inputBuf)-1] != '\n')
            {
                printf("Input too long. Please try again.\n");
                argument = NULL;
            }
        }

        cli_Port = atoi(argument);

        free(inputBuf);
    }

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(cli_Port);

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

    char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
    memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));
    
    while(1)
    {
        if (connect(clisoc, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
        {
             perror("Connection Error.\n");
             close(clisoc);
             return EXIT_FAILURE;
        }
        else
        {
            printf("Connection Successful.\n");
        }
        if ((re = read(clisoc, recbuff, sizeof(recbuff))) < 0)
        {
            perror("Read Error.\n");
            close(clisoc);
            return EXIT_FAILURE;
        }
        
        printf("Mesage from server: %s\n", recbuff);
        
        printf("Please enter a currency name.\n");

        fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
      
        inputBuf[strlen(inputBuf) - 1] = '\0';

        if (write(clisoc,inputBuf,DEFAULT_BUFFER_SIZE)<0)
        {
            perror("Error sending input to server.");
            close(clisoc);
            free(inputBuf);
            return EXIT_FAILURE;
        }

        if ((re = read(clisoc,recbuff,DEFAULT_BUFFER_SIZE)) > 0)
        {
            printf("Received from server: %s\n", recbuff);
            memset(recbuff,'\0', DEFAULT_BUFFER_SIZE);
        }
        else
        {
            perror("Error in server acknowledgement.");
            close(clisoc);
            free(inputBuf);
            return EXIT_FAILURE;
        }

        memset(inputBuf, '\0', DEFAULT_BUFFER_SIZE);

        printf("Please enter the password.\n");

        fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);

        inputBuf[strlen(inputBuf) - 1] = '\0';
        
        if (write(clisoc,inputBuf,DEFAULT_BUFFER_SIZE)<0)
        {
            perror("Error sending password to server.");
            close(clisoc);
            free(inputBuf);
            return EXIT_FAILURE;
        }

        int numRead = 0;

        memset(recbuff, '\0', DEFAULT_BUFFER_SIZE);

        while(numRead == 0)
        {
            numRead = read(clisoc,recbuff,DEFAULT_BUFFER_SIZE);
            if (numRead == 0)
            {
                continue;
            }
            else
            {
                if (numRead > 0)
                {
                    printf("Result: %s\n", recbuff);
                }
                else
                {
                    perror("Error getting response from server.");
                }
            }
        }
    }
        
        /*if(send(clisoc,argument,sizeof(argument,0))<0)
        {
            perror("Error sending input to server.");
            continue;
        }*/
        
    free(inputBuf);

    close(clisoc);

    return EXIT_SUCCESS;
}
