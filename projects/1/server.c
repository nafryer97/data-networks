#include"server.h"

int main(int argc, char** argv)
{
    /*
     * Berkeley sockets: http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
     */
    struct sockaddr_in sa;
    struct sockaddr_in cli;
    
    int socketfd;
    int conntfd;
    int len;
    int ch;

    int  cliPort;

    // Server needs to take an argument that specifies the port it is listening to.
    if (argc > 1)
    {
        //argv[0] is just the name of the executable
       cliPort  = atoi(argv[1]);
    }
    else
    {
        //
        //no command-line arguments were provided, ask for port
        //
        //https://www.geeksforgeeks.org/why-to-use-fgets-over-scanf-in-c/
        //
        char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
        memset(inputBuf, '\0', DEFAULT_BUFFER_SIZE);

        char* argument = NULL;
        
        printf("Please enter the port number.\n");
        
        while(argument == NULL)
        {
            argument = fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
            if (inputBuf[strlen(inputBuf)-1] != '\n')
            {
                printf("Input too long. Please try again.\n");
                argument = NULL;
            }
        }
        
        cliPort = atoi(argument);
        free(inputBuf);
    }   

    socketfd = socket(AF_INET,SOCK_STREAM,0);

    if (socketfd<0)
    {
        perror("Error in socket.\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Socket Openeed.\n");
    }

    //sets all bytes of sa to 0
    bzero(&sa,sizeof(sa));

    /*
     * https://jameshfisher.com/2016/12/21/htons/
     */
    sa.sin_port=htons(cliPort);
    sa.sin_addr.s_addr=htonl(0);

    if (bind(socketfd, (struct sockaddr*) &sa, sizeof(sa)) < 0 )
    {
        perror("Error in binding\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Bound Successfully\n");
    }

    if(listen(socketfd,50)<0)
    {
        perror("Error listening for connections.");
        return EXIT_FAILURE;
    }

    for(;;)
    {
        len=sizeof(ch);
        conntfd=accept(socketfd,(struct sockaddr*)&cli,&len);

        if (conntfd<0)
        {
            perror("Error accepting connection.\n");
            continue;
        }
        else
        {
            printf("Accepted.\n");
        }

        char greeting[]  = "Welcome! Please enter a command. \"goodbye\" to exit.";
            
        if(send(conntfd,greeting,sizeof(greeting),0)<0)
        {
            perror("Error sending greeting message.");
            close(conntfd);
            continue;
        }
        else
        {
           char* clientMsg = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
           memset(clientMsg, '\0', DEFAULT_BUFFER_SIZE);
           time_t timer = time(NULL);
           
           while (1)
           {
                int numRead = read(conntfd,clientMsg,DEFAULT_BUFFER_SIZE);
                if (numRead > 0)
                {
                    printf("Received from client: %s\n",clientMsg);
                    if(write(conntfd,clientMsg,DEFAULT_BUFFER_SIZE)<0)
                    {
                        perror("Error echoing client message.");
                        break;
                    }
                }

                memset(clientMsg, '\0', DEFAULT_BUFFER_SIZE);
           }
           free(clientMsg);
        }

        close(conntfd);
    }
    
    return EXIT_SUCCESS;
}
