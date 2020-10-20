#include"server.h"

int checkCurrency(char* input)
{
    char currencies[6][18] = {
                                "US Dollar",
                                "Canadian Dollar",
                                "Euro",
                                "British Pound",
                                "Japanese Yen",
                                "Swiss Franc"
    };

    int i=0;
    for(i=0; i<6; ++i)
    {
        //printf("Comparing %s and %s\n", input, currencies[i]);
        //printf("%i\n", strcmp(input, currencies[i]));
        if (strcmp(input, currencies[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

char* checkPassword(int currency, char* password)
{
    char passwords[6][9] = {
                                "uCh781fY",
                                "Cfw61RqV",
                                "Pd82bG57",
                                "Crc51RqV",
                                "wD82bV67",
                                "G6M7p8az"
    };

    char values[6][24] = {
                            "11081.00",
                            "14632.87",
                            "9359.20",
                            "8578.96",
                            "1158748.55",
                            "10100.44"
    };

    //printf("Comparing %s and %s\n", password, passwords[currency]);
    //printf("%i\n", strcmp(password, passwords[currency]));
    if (strcmp(password,passwords[currency]) == 0)
    {
        char* result = malloc(24);
        strcpy(result, values[currency]);
        return result;        
    }
    else
    {
        char* result = malloc(24);
        strcpy(result,"Passwords do not match.");
        return result;
    }

}

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
            perror("Error accepting connection.");
            continue;
        }
        else
        {
            printf("Accepted.\n");
        }
        
        char greeting[] = "hello client! this is server!";

        if(send(conntfd,greeting,sizeof(greeting),0)<0)
        {
            perror("Error sending greeting message.");
            close(conntfd);
            continue;
        }
        else
        {
           char* clientMsg = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
           char* currency = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
           char* password = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);

           memset(clientMsg, '\0', DEFAULT_BUFFER_SIZE);
           memset(currency, '\0', DEFAULT_BUFFER_SIZE);
           memset(password, '\0', DEFAULT_BUFFER_SIZE);
           
           time_t timer = time(NULL);

           int numRead = 0;           
           for(;;)
           {
               numRead = read(conntfd,clientMsg,DEFAULT_BUFFER_SIZE);
               
               if (numRead > 0)
               {
                   if(write(conntfd,clientMsg,numRead)<0)
                   {
                       perror("Error sending acknowledgement.");
                       break;
                   }

                   strcpy(currency, clientMsg);
                   break;
               }
           }

           memset(clientMsg, '\0', DEFAULT_BUFFER_SIZE);
            
           while((time(NULL) - timer) < 30)
           { 
               numRead = read(conntfd,clientMsg,DEFAULT_BUFFER_SIZE);
               
               if (numRead > 0)
               {
                  // if(write(conntfd,"Received.",10)<0)
                   //{
                   //    perror("Error sending acknowledgement.");
                   //}

                   strcpy(password, clientMsg);
                   break;
               }
           }

           if (strlen(password) > 0 && strlen(currency) > 0)
           {
               //printf("Input: %s Password: %s\n", currency, password);
               int match = checkCurrency(currency);
               if (match > -1)
               {
                    char* response = checkPassword(match,password);
                    if(write(conntfd,response,64) < 0)
                    {
                        perror("Error sending response.");
                    }
                    free(response);
               }
               else
               {
                    if(write(conntfd,"Currency did not match list of currencies.",64) < 0)
                    {
                        perror("Error sending response.");
                    }
               }
           }

           free(clientMsg);
           free(password);
           free(currency);
        }

        //write(conntfd,"Server Timed Out.",64);

        close(conntfd);
    }

    return EXIT_SUCCESS;
}
