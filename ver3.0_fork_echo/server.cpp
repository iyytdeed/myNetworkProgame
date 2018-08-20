#include<sys/socket.h>
#include<sys/types.h> // socket(..., types, ...)
#include<netinet/in.h> // sockadd_in; htons()
#include<arpa/inet.h> // inet_pton()
#include<stdio.h>
#include<unistd.h> // read()
#include<stdlib.h> // exit()
#include<strings.h> // bzero()
#include<string.h> // strlen()
#include<errno.h> // errno var
#include<fcntl.h> // fcntl()

#define MAXLINE 1000
#define LISTENQ 1024

void echo(int connfd)
{
    char buff[MAXLINE];
    int n;
    int flags;
    
    for(;;)
    {
        memset(buff, 0, MAXLINE);   //   rcv msg
        /* unblock mode
        while( (n = read(connfd, buff, MAXLINE)) < 0) // rcv msg
        {
            sleep(1);
        }
        
        if(n == 0) break; // EOF
        */
        
        /* block mode */
        if(flags = fcntl(connfd, F_GETFL, 0) < 0) // set block mode
    	{
        	printf("fcntl get error\n");
        	exit(1);
    	}
    	flags &= ~O_NONBLOCK;
    	if(fcntl(connfd, F_SETFL, flags) < 0)
    	{
        	printf("fcntl set error\n");
        	exit(1);
    	}
        
        n = read(connfd, buff, MAXLINE); // rcv msg
        if(n < 0)
        {
        	printf("read error\n");
        	exit(1);
        }
        if(n == 0) break; // EOF
        
        write(connfd, buff, n);	// echo msg
    }
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    struct sockaddr_in ser_addr;
    char buff[MAXLINE];
    
    int pid;
    
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
    
    // accept tcp-link and echo
    for(;;)
    //if(1) // can only serve once
    {
        if( (connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) // connect and will block
        {
            printf("connect error\n");
            exit(1);
        }
		
		if( (pid = fork()) == 0) /* child process */
		{
		    printf("\npid = %d, ppid = %d, begin\n", getpid(), getppid());
		    
			close(listenfd);
			
			echo(connfd); // forever loop
			
			if(close(connfd) < 0) // close connfd
        	{
            	printf("close error\n");
            	exit(1);
        	}
        	
        	printf("\npid = %d, ppid = %d, end\n", getpid(), getppid());
        	exit(0);
		}
        
        close(connfd);
    }
}
