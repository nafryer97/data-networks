#include"fryerSelectiveRepeatMessaging.h"

void redStdout(const char *msg)
{
    printf("\033[0;31m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void greenStdout(const char *msg)
{
    printf("\033[0;32m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void yellowStdout(const char *msg)
{
    printf("\033[0;33m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void blueStdout(const char *msg)
{
    printf("\033[0;34m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void magentaStdout(const char *msg)
{
    printf("\033[0;35m");
    printf("%s\n",msg);
    printf("\033[0m");
}

void cyanStdout(const char *msg)
{
    printf("\033[0;36m");
    printf("%s\n",msg);
    printf("\033[0m");
}

int usage(char *arg1, char *arg2)
{
    fprintf(stderr, "Usage: %s %s\n",arg1,arg2);
    return EXIT_SUCCESS;
}

void handleFatalErrorNo(int en, const char *msg, int sockfd)
{
    fprintf(stderr, "\033[1;31m");

    if (sockfd >= 0)
    {
        close(sockfd);
        fprintf(stderr, "Socket closed. ");
    }
    
    fprintf(stderr, "%s: %s\n",msg,strerror(en)); 
    
    exit(EXIT_FAILURE);
}

int handleErrorNoRet(int en, int retval, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s: %s\n", msg, strerror(en));
    fprintf(stderr, "\033[0m");
    return retval;
}

void handleErrorNoMsg(int en, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s: %s\n", msg, strerror(en));
    fprintf(stderr, "\033[0m");
}

void handleFatalError(const char *msg, int sockfd)
{
    fprintf(stderr, "\033[1;31m");
    
    if (sockfd >= 0)
    {
        close(sockfd);
        fprintf(stderr, "Socket closed. ");
    }
    
    fprintf(stderr, "%s\n",msg); 
    
    exit(EXIT_FAILURE);
}

int handleErrorRet(int retval, const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s\n", msg);
    fprintf(stderr, "\033[0m");
    return retval;
}

void handleErrorMsg(const char *msg)
{
    fprintf(stderr, "\033[1;31m");
    fprintf(stderr, "%s\n", msg);
    fprintf(stderr, "\033[0m");
}
