#include"server.h"

#ifndef CLI_PORT
#define CLI_PORT 50051
#endif

#ifndef BUFF_SIZE
#define BUFF_SIZE 30
#endif

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
        printf("Error in socket.\n");
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
        printf("Error in binding\n");
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
        printf("Accepted.\n");
        tick=time(NULL);
        snprintf(str,sizeof(str),"%s",ctime(&tick));
        //printf("%s",str);
        write(conntfd,str,100);
    }
    

    return EXIT_SUCCESS;
}
