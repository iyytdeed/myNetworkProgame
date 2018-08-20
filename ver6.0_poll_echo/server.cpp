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
#include<poll.h> // poll()
#include<limits.h> // OPEN_MAX

#define INFTIM -1
#define OPEN_MAX 1000
#define MAXLINE 1000
#define LISTENQ 1024

void err_exit(const char * str)
{
	printf("%s", str);
	exit(1);
}

int main(int argc, char **argv)
{
    int listenfd, connfd, sockfd, i, maxi, n;
    int nready;
	struct pollfd client[OPEN_MAX]; // fd, events, revents
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
   
    // init for poll paras
    client[0].fd = listenfd;
    client[0].events = POLLRDNORM; // read norm - event
    for(i = 1; i < OPEN_MAX; i++)   client[i].fd = -1;
    maxi = 0;
    // accept tcp-link and echo
    for(;;)
    {
        // poll block
        if( (nready = poll(client, maxi + 1, INFTIM)) < 0) // Inf-time means block forever
            err_exit("poll error\n");
        
        // case 1. accept
        if(client[0].revents & POLLRDNORM)
        {
        	clilen = sizeof(cli_addr);
        	if( (connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &clilen)) < 0) // accept
        		err_exit("accept error\n");
        	
        	for(i = 1; i < OPEN_MAX; i++) // regist connfd in client[]
        	{
        		if(client[i].fd == -1)
        		{
        			client[i].fd = connfd;
        			client[i].events = POLLRDNORM;
        			break;
        		}
        	}
        	if(i == OPEN_MAX)
        		err_exit("client[] full!!\n");
        	
        	if(i > maxi)	maxi = i;
        	
        	if(--nready <= 0) continue; // no more select info
        }
        
        // case 2. msg reach
        for(i = 1; i < OPEN_MAX; i++)
        {
            if( (sockfd = client[i].fd) < 0) continue;
            if(client[i].revents & (POLLRDNORM | POLLERR))
            {
                if( (n = read(sockfd, buff, MAXLINE)) < 0)
                {
                    if(errno == ECONNRESET) // reset err
                    {
                        if(close(sockfd) < 0)
                            err_exit("close error\n");
                        client[i].fd = -1;
                    }
                    else // unkown err
                    {
                        err_exit("read unkown error\n");
                    }
                }
                else if(n == 0) // EOF
                {
                    if(close(sockfd) < 0)
                        err_exit("close error\n");
                    client[i].fd = -1;
                }
                else
                {
                    if(write(sockfd, buff, n) < 0)
                        err_exit("echo error\n");
                }
                
                if(--nready <= 0) break;
            }
        }
    }
}
