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

void sig_chld(int signo)
{
	int pid;
	int stat;
	
	while( (pid = waitpid(-1, &stat, WNOHANG)) > 0) // one singal for many zombies , till process all
	{
		printf("pid %d is terminated\n", pid);
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
   
	// signal chld
	signal(SIGCHLD, sig_chld);

    // accept tcp-link and echo
    for(;;)
    {
        if( (connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) // connect and will block
        {
			if(errno == EINTR) continue; // INTR error can ignore

            printf("connect error\n"); // else error should exit
            exit(1);
        }
		
		if( (pid = fork()) == 0) /* child process */
		{
		    printf("\npid = %d, ppid = %d, begin\n", getpid(), getppid());
		    
			close(listenfd);
			
			echo(connfd); // forever loop
			
			if(close(connfd) < 0) // close connfd
        	{
            	printf("child close connfd error\n");
            	exit(1);
        	}
        	
        	printf("\npid = %d, ppid = %d, end\n", getpid(), getppid()); // close child process
        	exit(0);
		}
       
		if(close(connfd) < 0) /* father process */
		{
			printf("father close connfd error\n");
			exit(1);
		}
    }
}
