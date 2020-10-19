#include<stdio.h> 
#include<sys/types.h> 
#include<sys/socket.h> 
#include<netinet/in.h> 
#include<time.h> 
#include<unistd.h>
#include<string.h> 
#include<arpa/inet.h> 
#include<stdlib.h>

#define CLI_PORT 2100
#define BUFF_SIZE 30

int main(int argc,char **argv) 
{
	int clisoc,re;
	char recbuff[BUFF_SIZE];
	struct sockaddr_in cliaddr;
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family=AF_INET;
	cliaddr.sin_port=htons(CLI_PORT); 
	cliaddr.sin_addr.s_addr=inet_addr(argv[1]);
	
	if((clisoc=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("\nSocket Errror");
		exit(0);
	}
	else
	{ 
		printf("\nSocket opened");
	}

	if((connect(clisoc,(struct sockaddr *)&cliaddr,sizeof(cliaddr)))<0) 
	{
		perror("\nConnect Error");
		exit(0);
	}
	else
	{ 
		printf("\nConnected Successfully");
	} 
	if((re=(read(clisoc,recbuff,sizeof(recbuff))))<0)
	{
		printf("Read Error");
		exit(0);
	}
	printf("\nThe Current Date and Time is %s\n",recbuff); 
	close(clisoc);
	return 0;
}
