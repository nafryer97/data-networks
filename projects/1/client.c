#include"client.h"

int readFromServer(int clisoc)
{
    int nread = 0;
    int errsv = 0;

    char* sbuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(sbuf, '\0', DEFAULT_BUFFER_SIZE);
    
    do {
        nread = recv(clisoc,sbuf,(DEFAULT_BUFFER_SIZE-1),0);
            
        if (nread < 0)
        {
            perror("Error receiving response from server.");
            errsv = errno;
            free(sbuf);
            return errsv;
        }

    } while(nread == 0);
    
    if (nread > 0)
    {
        printf("From Server: %s\n", sbuf);
    }
   
    free(sbuf);
    return 0;
}

int sendToServer(int clisoc, char* str)
{
    int errsv = 0;
    
    if (send(clisoc,str,(strlen(str)+1),0)<0)
    {
        perror("Error sending input to server.");
        errsv = errno;
        return errsv;
    }

    return 0;
}

int currencyProgram(int clisoc, int* errsv)
{
    int nread = 0;
    
    char* ibuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
    memset(ibuf, '\0', DEFAULT_BUFFER_SIZE);

    /*
     * 1. Prompt with welcome message after successful connection. Get input from the keyboard and 
     * send it to the server.
     */

    printf("Please enter a currency name.\n");

    fgets(ibuf, DEFAULT_BUFFER_SIZE, stdin);

    removeNewLine(ibuf);

    char* message = strndup(ibuf, (strlen(ibuf)+1));

    if ( ( *errsv = sendToServer(clisoc, message)) != 0)
    {
        free(message);
        free(ibuf);
        return -1;
    }

    free(message);

    /*
     * 2. Server should acknowledge receipt of currency input from client.
     */

    if ( (*errsv = readFromServer(clisoc)) != 0)
    {
        free(ibuf);
        return -1;
    }

    memset(ibuf, '\0', DEFAULT_BUFFER_SIZE);

    /*
     * 3. The client will ask for the user's password after it has received 
     * acknowledgement from the server. The password is sent to the server.
     */

    printf("Please enter the password.\n");

    fgets(ibuf, DEFAULT_BUFFER_SIZE, stdin);

    removeNewLine(ibuf);

    message = strndup(ibuf, (strlen(ibuf)+1));

    if ( ( *errsv = sendToServer(clisoc, message)) != 0)
    {
        free(message);
        free(ibuf);
        return -1;
    }

    free(message);

    /*
     * 4. Server will verify password and currency against list of legitimate pairs.
     * If there is a match, the server will send the corresponding bitcoin value. Otherwise,
     * the server will send a failure message.
     */
    
    if ( ( *errsv = readFromServer(clisoc)) != 0)
    {
        free(ibuf);
        return -1;
    }
        
    free(ibuf);

    return 0;
}

void removeNewLine(char* str)
{
    char* nl = strchr(str,'\n');
    if (nl != NULL)
    {
        *nl='\0';
    }
}

char* getServerAddressCli()
{
    char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
    memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));

    char* argument = NULL;
        
    printf("Please enter the server's ip address.\n");
        
    while(argument == NULL)
    {
        argument = fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
        
        if (*(inputBuf + (strlen(inputBuf) - 1)) != '\n')
        {
            printf("Input too long. Please try again.\n");
            argument = NULL;
            memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));
        }
    }
    
    removeNewLine(inputBuf);
    argument = strndup(inputBuf, (strlen(inputBuf)+1));
    
    free(inputBuf);
    
    return argument;
}

int getPortNumCli()
{   //
    //no command-line arguments were provided, ask for port
    //
    //https://www.geeksforgeeks.org/why-to-use-fgets-over-scanf-in-c/
    //
    int cliPort = 0;
    char* inputBuf = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE); 
    memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));

    char* argument = NULL;
        
    printf("Please enter the port number.\n");
        
    while(argument == NULL)
    {
        argument = fgets(inputBuf, DEFAULT_BUFFER_SIZE, stdin);
        
        if (*(inputBuf + (strlen(inputBuf) - 1)) != '\n')
        {
            printf("Input too long. Please try again.\n");
            argument = NULL;
            memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));
        }
        else if (atoi(argument) < DEFAULT_MIN_PORT || atoi(argument) > DEFAULT_MAX_PORT)
        {
            printf("Must use one of ports %i-%i.\n",DEFAULT_MIN_PORT,DEFAULT_MAX_PORT);
            argument=NULL;
            memset(inputBuf, '\0', (sizeof(char) * DEFAULT_BUFFER_SIZE));
        }
    }
   
    removeNewLine(argument); 
    
    cliPort = atoi(argument);
    
    free(inputBuf);

    return cliPort;
}

int main(int argc, char** argv)
{ 
    int clisoc;
    int re;
    int cli_Port;

    char* ipAddress;

    char recbuff[DEFAULT_BUFFER_SIZE];
    
    struct sockaddr_in cliaddr;

    memset(&cliaddr,0,sizeof(struct sockaddr_in));
    
    /*
     * Client needs to either take server ip address and port command-line 
     * arguments, or it needs to ask for them once it starts.
     */
    if (argc > 2)
    {
        //first arg is just the name of the executable
        //second arg should be ip address of server
        ++argv;
        removeNewLine(*argv);
        ipAddress = strndup(*argv, (strlen(*argv)+1));

        //second argument should be port that server is listening to
        ++argv;
        removeNewLine(*argv);
        int test = atoi(*argv);
        if(test < DEFAULT_MIN_PORT || test > DEFAULT_MAX_PORT)
        {
            printf("Invalid port number: %s\n", *argv);
            cli_Port = getPortNumCli();
        }
        else
        {
            cli_Port = test;
        }
    }
    else
    {
        //no command-line arguments were provided, ask for server and port
        ipAddress = getServerAddressCli();
        cli_Port = getPortNumCli();

    }   
    
    printf("Server address %s\n", ipAddress);
    printf("Using port %i\n", cli_Port);

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

    for(;;)
    {
        if (connect(clisoc, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
        {
             perror("Connection Error.\n");
             sleep(2);
             continue;
        }
        else
        {
            printf("Connection Successful.\n");
        }

        int errsv = 0;

        if(currencyProgram(clisoc, &errsv)<0)
        {
            printf("%s\n", strerror(errsv));
            break;
        }
        else
        {
            printf("Goodbye!\n");
            break;
        }
        
    }

    free(ipAddress);

    return EXIT_SUCCESS;
}
