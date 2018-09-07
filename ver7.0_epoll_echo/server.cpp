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
#include<sys/epoll.h> // epoll()

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
	socklen_t clilen;
    sockaddr_in ser_addr, cli_addr;
    char buff[MAXLINE];
    epoll_event event, events[OPEN_MAX];
    int epfd;
    
    // get socket(for listen)
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("sock error\n");
        exit(1);
    }
    
    // set addr
    bzero(&ser_addr, sizeof(ser_addr));
    bzero(&cli_addr, sizeof(cli_addr));
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
   
    // init for epoll paras
	// epfd = epoll_create(int size);
    epfd = epoll_create1(0); // flags - 0
    bzero(&event, sizeof(epoll_event));
    event.events = EPOLLIN;
    event.data.fd = listenfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event) < 0)
		err_exit("epoll_ctl error\n");
	
    // accept tcp-link and echo
    for(;;)
    {
		nready = epoll_wait(epfd, events, OPEN_MAX, -1);
		if(nready < 0)
			err_exit("epoll_wait error\n");
		
		for(i = 0; i < nready; i++)
		{
			if(events[i].data.fd == listenfd) // should accept
			{
				clilen = sizeof(cli_addr);
				if( (connfd = accept(listenfd, (sockaddr *) &cli_addr, &clilen)) < 0)
					err_exit("accept error\n");
				
				bzero(&event, sizeof(epoll_event));
				event.events = EPOLLIN;
				event.data.fd = connfd;
				if(epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event) < 0)
					err_exit("epoll_ctl error\n");
			}
			else if(events[i].events & EPOLLIN) // should echo
			{
				sockfd = events[i].data.fd;
				if( (n = read(sockfd, buff, MAXLINE)) < 0)
				{
					if(errno == ECONNRESET) // rst err
					{
						if(close(sockfd) < 0)
							err_exit("close connfd error\n");
						
						bzero(&event, sizeof(epoll_event));
						event.events = EPOLLIN;
						event.data.fd = sockfd;
						epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &event);
					}
					else
						err_exit("read unkown error\n");
				}
				else if(n == 0)
				{
					if(close(sockfd) < 0)
						err_exit("close error\n");
					
					bzero(&event, sizeof(epoll_event));
					event.events = EPOLLIN;
					event.data.fd = sockfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &event);
				}
				else
				{
					if(write(sockfd, buff, n) < 0)
						err_exit("echo error\n");
				}
			}
		}
		
    }
}
