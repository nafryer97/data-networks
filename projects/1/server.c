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

    char str[100];

    time_t tick;

    //system call
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
    sa.sin_port=htons(CLI_PORT);
    sa.sin_addr.s_addr=htonl(0);

    if (bind(socketfd, (struct sockaddr*) &sa, sizeof(sa)) < 0 )
    {
        perror("Error in binding\n");
    }
    else
    {
        printf("Bound Successfully\n");
    }

    listen(socketfd,50);

    for(;;)
    {
        len=sizeof(ch);
        conntfd=accept(socketfd,(struct sockaddr*)&cli,&len);

        if (conntfd<0)
        {
            perror("Error accepting connection.\n");
            return EXIT_FAILURE;
        }
        else
        {
            printf("Accepted.\n");
        }

        tick=time(NULL);
        snprintf(str,sizeof(str),"%s",ctime(&tick));
        //printf("%s",str);
        if(write(conntfd,str,100)<0)
        {
            perror("Error writing to socket file descriptor.\n");
            return EXIT_FAILURE;
        }
        else
        {
            return EXIT_SUCCESS;
        }
    }
    return EXIT_SUCCESS;
}
