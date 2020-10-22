#include"server.h"

int checkCurrency(char* input)
{
    const char* currencies[6];
    currencies[0] = "US Dollar";
    currencies[1] = "Canadian Dollar";
    currencies[2] = "Euro";
    currencies[3] = "British Pound";
    currencies[4] = "Japanese Yen";
    currencies[5] = "Swiss Franc";

    int i=0;
    for(i=0; i<6; ++i)
    {
        if (strcmp(input, currencies[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

char* checkPassword(int currency, char* password)
{
    const char* passwords[6]; 
    passwords[0] = "uCh781fY";
    passwords[1] = "Cfw61RqV";
    passwords[2] = "Pd82bG57";
    passwords[3] = "Crc51RqV";
    passwords[4] = "wD82bV67";
    passwords[5] = "G6M7p8az";

    const char* values[6];
    values[0] = "11081.00";
    values[1] = "14632.87";
    values[2] = "9359.20";
    values[3] = "8578.96";
    values[4] = "1158748.55";
    values[5] = "10100.44";

    char* result;
    
    if (currency < 0 || currency > 5)
    {
        result = strdup("Invalid Currency.");
    }
    else if (strcmp(password,passwords[currency]) == 0)
    {
        result = strdup(values[currency]);
    }
    else
    {
        result = strdup("Passwords do not match.");
    }

    return result;
}

void removeNewLine(char* str)
{
    char* nl = strchr(str,'\n');
    if (nl != NULL)
    {
        *nl='\0';
    }
}

int getPortNumCli()
{   
    /* 
     * no command-line arguments were provided, ask for port
     * https://www.geeksforgeeks.org/why-to-use-fgets-over-scanf-in-c/
     */

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

int openSocket()
{
    int socketfd = socket(AF_INET,SOCK_STREAM,0);

    if (socketfd<0)
    {
        perror("Error in socket.\n");
    }
    else
    {
        printf("Socket Opened.\n");
    }

    return socketfd;
}

int bindPort(int socketfd, int cliPort, struct sockaddr_in* sa)
{
    //sets all bytes of sa to 0
    memset(sa,0,sizeof(struct sockaddr_in));

    /*
     * https://jameshfisher.com/2016/12/21/htons/
     */
    (*sa).sin_port=htons(cliPort);
    (*sa).sin_addr.s_addr=htonl(0);

    if (bind(socketfd, (struct sockaddr*) sa, sizeof(struct sockaddr_in)) < 0 )
    {
        perror("Error in binding\n");
        return -1;
    }
    else
    {
        printf("Bound Successfully\n");
        return 0;
    }
}

void processInput(int clientfd, char* currency, char* password)
{
    int match = checkCurrency(currency);
    if (match > -1)
    {
        char* response = checkPassword(match,password);
        if(send(clientfd,response,strlen(response),0) < 0)
        {
            perror("Error sending response.");
        }
        free(response);
    }
    else
    {
        char response[] = "Currency did not match list of currencies.";
        
        if(send(clientfd,response,strlen(response),0) < 0)
        {
            perror("Error sending response.");
        }
    }
}

char* readCurrency(int clientfd)
{
    char* clientMsg = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(clientMsg, '\0', DEFAULT_BUFFER_SIZE);

    time_t timer = time(NULL);
    
    char* currency = NULL;

    while(((int)(time(NULL) - timer)) < 30)
    {
        int numRead = recv(clientfd,clientMsg,(DEFAULT_BUFFER_SIZE-1),MSG_DONTWAIT);
        
        if (numRead > 0)
        {
            if(send(clientfd,clientMsg,numRead,0)<0)
            {
                perror("Error sending acknowledgement.");
            }

            currency = strdup(clientMsg);
            break;
        }
        else if (numRead < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                sleep(1);
                continue;
            }

            perror("Error reading from client.");
            break;
        }
    }

    if (((int)(time(NULL)) - timer) >= 30)
    {
        fprintf(stdout, "Timed out waiting on currency.\n");
    }

    free(clientMsg);
    return currency;
}

char* readPassword(int clientfd)
{
    char* clientMsg = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(clientMsg, '\0', DEFAULT_BUFFER_SIZE);

    time_t timer = time(NULL);
    
    char* password = NULL;

    while(((int)(time(NULL)) - timer) < 30)
    { 
        int numRead = recv(clientfd,clientMsg,(DEFAULT_BUFFER_SIZE-1),MSG_DONTWAIT);
        if (numRead > 0)
        {
            password = strdup(clientMsg);
            break;
        }
        else if (numRead < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                sleep(1);
                continue;
            }
            perror("Error reading from client.");
            break;
        }
    }

    if (((int)(time(NULL)) - timer) >= 30)
    {
        fprintf(stdout, "Timed out waiting on password.\n");
    }

    free(clientMsg);
    return password;
}

int currencyProgram(int clientfd)
{
    char* currency = NULL;
    char* password = NULL;

    for(;;)
    {
        currency = readCurrency(clientfd);
        
        if(!currency)
        {
            break;
        }
        
        password = readPassword(clientfd);

        if (currency)
        {
            if (password)
            {
                processInput(clientfd, currency, password);
                free(password);
            }
            
            free(currency);
        }
        else if (!currency && password)
        {
            free(password);    
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    /*
     * Berkeley sockets: http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
     */
    struct sockaddr_in sa;
    struct sockaddr_in cli;
    
    int socketfd;
    int clientfd;
    int len;

    int cliPort;
    
    if (argc > 1)
    {
        ++argv;
        removeNewLine(*argv);
        int test = atoi(*argv);
        if(test < DEFAULT_MIN_PORT || test > DEFAULT_MAX_PORT)
        {
            printf("Invalid port number: %s\n", *argv);
            cliPort = getPortNumCli();
        }
        else
        {
            cliPort = test;
        }
    }
    else
    {
        cliPort = getPortNumCli(); 
    }   
    
    printf("Using port %i\n", cliPort);
    socketfd = openSocket();

    if (bindPort(socketfd, cliPort, &sa) < 0)
    {
        close(socketfd);
        return EXIT_FAILURE;
    }

    if(listen(socketfd,50)<0)
    {
        perror("Error setting socket to listen.");
        return EXIT_FAILURE;
    }

    for(;;)
    {
        len=sizeof(struct sockaddr_in);
        clientfd=accept(socketfd,(struct sockaddr*)&cli,&len);

        if (clientfd<0)
        {
            perror("Error accepting connection.");
            continue;
        }
        else
        {
            printf("Accepted.\n");
            currencyProgram(clientfd);        
        }
 
        close(clientfd);
        printf("Connection Closed. Waiting for new connection...\n");
    }

    return EXIT_SUCCESS;
}
