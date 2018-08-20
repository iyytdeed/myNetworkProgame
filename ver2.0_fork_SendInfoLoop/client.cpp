#include<sys/socket.h> // socket
#include<sys/types.h> // for para type of socket()
#include<netinet/in.h> // for struct sockaddr_in ; for htons()
#include<arpa/inet.h> // for inet_pton()
#include<strings.h> // for bzero()
#include<stdio.h>
#include<unistd.h> // for read()
#include<stdlib.h> // exit()

#define MAXLINE 1000

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
    
    // read from tcp-link
    for(;;)
    {
        n = read(sockfd, rcvline, MAXLINE);
        if(n < 0)
        {
            printf("read error\n");
            exit(1);
        }
        else
        {
            rcvline[n] = 0; // null terminate
            if( fprintf(stdout, "%s", rcvline) == EOF)
            {
                printf("fprintf rcvline error\n");
                exit(1);
            }
        }
        sleep(0.5);
    }
    
    return 0;
}

