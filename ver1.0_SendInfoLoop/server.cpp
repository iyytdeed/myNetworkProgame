// can only serve for one client

#include<sys/socket.h>
#include<sys/types.h> // socket(..., types, ...)
#include<netinet/in.h> // sockadd_in; htons()
#include<arpa/inet.h> // inet_pton()
#include<stdio.h>
#include<unistd.h> // read()
#include<stdlib.h> // exit()
#include<strings.h> // bzero()
#include<string.h> // strlen()

#include<time.h>

#define MAXLINE 1000
#define LISTENQ 1024

int main(int argc, char **argv)
{
    int listenfd, connfd;
    struct sockaddr_in ser_addr;
    char buff[MAXLINE];
    time_t ticks;
    
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
    {
        if( (connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0) // connect
        {
            printf("connect error\n");
            exit(1);
        }
        
        pid_t pid = getpid();
        int cnt = 0;
        for(;;)
        {
            ticks = time(NULL); // get time and write into link
            snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
            if(write(connfd, buff, strlen(buff)) < 0)
            {
                printf("write error\n");
                exit(1);
            }
            printf("pid = %d; loop - %d\n", pid, cnt++);
            sleep(2);
        }
        
        if(close(connfd) < 0)  // close link
        {
            printf("close error\n");
            exit(1);
        }
    }
}
