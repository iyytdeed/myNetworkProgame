#include<arpa/inet.h> // inet_pton()
#include<stdio.h>
#include<unistd.h> // read()
#include<stdlib.h> // exit()
#include<strings.h> // bzero()
#include<string.h> // strlen()
#include<errno.h> // errno var
#include<fcntl.h> // fcntl()
#include<pthread.h> // 

#define MAXLINE 1000
#define LISTENQ 1024

static void* echo(void *arg)
{
    char buff[MAXLINE];
    int n;
    int flags;
    int connfd = *((int *)arg);
    
    printf("thread %lu is begin\n", pthread_self());
    pthread_detach(pthread_self()); // detach
    
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
    
    for(;;)
    {
        memset(buff, 0, MAXLINE);   //   rcv msg
        /* unblock mode
        while( (n = read(connfd, buff, MAXLINE)) < 0) // rcv msg
        {
            sleep(1);
        }
		*/
        
        n = read(connfd, buff, MAXLINE); // rcv msg
        if(n < 0)
        {
        	printf("read error\n");
        	exit(1);
        }

        if(n == 0) break; // EOF
        
        write(connfd, buff, n);	// echo msg
    }
    
    close(connfd);
    printf("thread %lu is end\n", pthread_self());
    
    return NULL;
}


int main(int argc, char **argv)
{
    int listenfd, connfd;
    struct sockaddr_in ser_addr;
    char buff[MAXLINE];
    
    pthread_t tid;
    
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
   
    printf("main thread is %lu \n", pthread_self());
    // accept tcp-link and echo
    for(;;)
    {
        if( (connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) // connect and will block
        {
			if(errno == EINTR) continue; // INTR error can ignore

            printf("connect error\n"); // else error should exit
            exit(1);
        }
		
		pthread_create(&tid, NULL, echo, (void*) &connfd);
    }
}
