#include <netinet/in.h> // for struct sockaddr_in ; for htons()
#include <arpa/inet.h>  // for inet_pton()
#include <stdio.h>
#include <unistd.h> // for read()
#include <stdlib.h> // exit()
#include <string.h> // strlen()
#include <fcntl.h>
#define MAXLINE 1000

void cli_echo(int sockfd)
{
    char send_buff[MAXLINE];
    char recv_buff[MAXLINE];
    int n;

    memset(send_buff, 0, MAXLINE);
    while (true)
    {
        sleep(1);
        memset(send_buff, 0, MAXLINE);
        strcpy(send_buff, "hello world\n");   
        send(sockfd, send_buff, strlen(send_buff), 0);

        memset(recv_buff, 0, MAXLINE);
        n = recv(sockfd, recv_buff, MAXLINE, 0);
        if (n < 0)
        {
            printf("n < 0 - recv echoinfo error\n");
            close(sockfd);
            exit(1);
        }
        else if (n == 0)
        {
            printf("n = 0 - server terminated premuturely\n");
            close(sockfd);
            exit(1);
        }
        else
        {
            printf("%s\n", recv_buff);
        }
    }
}

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in ser_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(2018);
    inet_pton(AF_INET, "127.0.0.1", &ser_addr.sin_addr);

    int flags = fcntl(sockfd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);
    connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));

    cli_echo(sockfd);

    return 0;
}