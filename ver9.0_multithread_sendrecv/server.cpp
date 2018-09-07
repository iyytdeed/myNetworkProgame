#include <arpa/inet.h> // inet_pton()
#include <stdio.h>
#include <unistd.h> // close()
#include <stdlib.h> // exit()
#include <string.h> // strlen()
#include <errno.h>  // errno var
#include <pthread.h>
#include <fcntl.h>

#define LISTEN_MAX 1024
#define MAXLINE 1000

void *
thr1(void *arg)
{
    char buffer[MAXLINE];
    int connfd = *((int *)arg);
    int n;

    pthread_detach(pthread_self());

    for (;;)
    {
        memset(buffer, '\0', MAXLINE);
        n = recv(connfd, buffer, MAXLINE, 0);
        if (n > 0)
        {
            printf("connfd %d - ", connfd);
            printf("%s\n", buffer);
            send(connfd, buffer, MAXLINE, 0);
        }
        else if (n == 0)
        {
            printf("link over\n");
            close(connfd);
            return NULL;
        }
        else
        {
            printf("error\n");
            exit(1);
        }
    }

    close(connfd);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; // ipv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(2018);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(listenfd, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(listenfd, F_SETFL, flags);

    bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));

    listen(listenfd, LISTEN_MAX);

    printf("main thread is %lu \n", pthread_self());
    pthread_t tid;
    for (;;)
    {
        struct sockaddr_in cli_addr;
        socklen_t socklen;
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &socklen);
        int flags = fcntl(listenfd, F_GETFL, 0);
        flags &= ~O_NONBLOCK;
        fcntl(listenfd, F_SETFL, flags);
        pthread_create(&tid, NULL, thr1, (void *)&connfd);
    }

    return 0;
}
