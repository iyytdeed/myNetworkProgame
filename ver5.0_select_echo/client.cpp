#include<sys/socket.h> // socket
#include<sys/types.h> // for para type of socket()
#include<netinet/in.h> // for struct sockaddr_in ; for htons()
#include<arpa/inet.h> // for inet_pton()
#include<strings.h> // for bzero()
#include<stdio.h>
#include<unistd.h> // for read()
#include<stdlib.h> // exit()
#include<string.h> // strlen()
#include<fcntl.h> // fcntl()

#define MAXLINE 1000

void cli_sendstr(FILE *fp, int sockfd)
{
	char send_buff[MAXLINE];
	char recv_buff[MAXLINE];
	int flags;
	int n;
	
	/* set block mode */
    if(flags = fcntl(sockfd, F_GETFL, 0) < 0) // set block mode
   	{
       	printf("fcntl get error\n");
       	exit(1);
   	}
   	flags &= ~O_NONBLOCK;
   	if(fcntl(sockfd, F_SETFL, flags) < 0)
   	{
       	printf("fcntl set error\n");
       	exit(1);
   	}
	
	memset(send_buff, 0, MAXLINE);
	while(fgets(send_buff, MAXLINE, fp) != NULL)    //  read from command line
	{
		write(sockfd, send_buff, strlen(send_buff)); // send
		
		memset(recv_buff, 0, MAXLINE);
        n = read(sockfd, recv_buff, MAXLINE); // read echoinfo
        if(n < 0)
        {
            printf("recv echoinfo error\n");
            exit(1);
        }
	    if(n == 0)
		{
			printf("n = 0 - server terminated premuturely\n");
			exit(1);
		}

		fputs(recv_buff, stdout);   // show echo to command line
		
		memset(send_buff, 0, MAXLINE);
	}
	
}

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in ser_addr;
    char rcvline[MAXLINE + 1];
    
    if(argc != 1)
    {
        printf("usage: a.out\n");
        exit(1);
    }
    
    // get socket
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // IPv4, stream type, protocal-0-default
    {
        printf("socket error\n");
        exit(1);
    }
    
    // set addr
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(9990);
    if(inet_pton(AF_INET, "127.0.0.1", &ser_addr.sin_addr) <= 0)
    {
        printf("inet_pton error\n");
        exit(1);
    }
    
    // connect - nonblock - 2s 1time
    while(connect(sockfd, (struct sockaddr*) &ser_addr, sizeof(ser_addr)) < 0)
    {
        printf("connect error\n");
        sleep(2);
        //exit(1);
    }
    
    // send to link and recv echo info
    cli_sendstr(stdin, sockfd);
    
    return 0;
}

