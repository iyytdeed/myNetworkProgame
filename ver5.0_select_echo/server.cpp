#include<arpa/inet.h> // inet_pton()
#include<stdio.h>
#include<unistd.h> // read()
#include<stdlib.h> // exit()
#include<strings.h> // bzero()
#include<string.h> // strlen()
#include<errno.h> // errno var
#include<fcntl.h> // fcntl()
#include<signal.h> // signal()
#include<sys/wait.h> // waitpid()
#include<sys/select.h> // select()
#include<sys/time.h> // select(..., timeval )

#define MAXLINE 1000
#define LISTENQ 1024

void err_exit(const char * str)
{
	printf("%s", str);
	exit(1);
}

int main(int argc, char **argv)
{
    int listenfd, connfd, i, maxi, maxfd, sockfd, n;
    int nready, client[FD_SETSIZE];
	fd_set rset, allset;
	socklen_t clilen;
    struct sockaddr_in ser_addr, cli_addr;
    char buff[MAXLINE];   

    // get socket(for listen)
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("sock error\n");
        exit(1);
    }
    
    // set addr
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); // differ from client
    ser_addr.sin_port = htons(9990);
    
    // bind
    if(bind(listenfd, (struct sockaddr*) &ser_addr, sizeof(ser_addr)) < 0)
    {
        printf("bind error\n");
        exit(1);
    }
    
    // listen
    if(listen(listenfd, LISTENQ) < 0)
    {
        printf("listen error\n");
        exit(1);
    }
   
    // init for select paras
    maxfd = listenfd;
    maxi = -1; // index for client[]
    for(i = 0; i < FD_SETSIZE; i++) client[i] = -1; // -1 indicates available entry
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
   
    // accept tcp-link and echo
    for(;;)
    {
        rset = allset; // structure assignment ( allset will update during the for loop )
        nready = select(maxfd+1, &rset, NULL, NULL, NULL); // time null means block forever
        
        // case 1. get new connfd
        if(FD_ISSET(listenfd, &rset))
        {
        	clilen = sizeof(cli_addr);
        	if( (connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &clilen)) < 0) // accept
        		err_exit("accept error\n");
        	
        	for(i = 0; i < FD_SETSIZE; i++) // regist connfd in client[]
        	{
        		if(client[i] == -1)
        		{
        			client[i] = connfd;
        			break;
        		}
        	}
        	if(i == FD_SETSIZE)
        		err_exit("client[] full!!\n");
        	
        	FD_SET(connfd, &allset); // add connfd to allset ,so that we can check inforeach of connfd with select
        	if(connfd > maxfd)	maxfd = connfd;
        	if(i > maxi)	maxi = i;
        	
        	if(--nready <= 0) continue; // no more select info
        }
        
        // case 2. get new msg
        for(i = 0; i <= maxi; i++) // check all client_socket and findout whoes msg is reached
        {
        	if( (sockfd = client[i]) < 0) continue;
        	if(FD_ISSET(sockfd, &rset)) 
        	{
        		if( (n = read(sockfd, buff, MAXLINE)) < 0)
        			err_exit("read error\n");
        		else if(n == 0)
        		{
        			if(close(sockfd) < 0) 
        				err_exit("close error\n");
        			FD_CLR(sockfd, &allset);
        			client[i] = -1;
        		}
        		else
        		{
        			if(write(sockfd, buff, n) < 0)
        				err_exit("echo error\n");
        		}
        		
        		if(--nready <= 0) break; // no more select info
        	}
        }
        
    }
}
