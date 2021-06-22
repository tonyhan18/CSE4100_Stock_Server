/* 
 * echoserveri.c - An iterative echo server 
 */
/* $begin echoserverimain */
#include "csapp.h"

struct item
{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
}

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}

int main(int argc, char **argv)
{
    int listenfd; //sockfd -> save the server socket
    int connfd; //new_fd
    int numbytes;
    char buf[MAXBUF];
    struct addrinfo hints, *servinfo;
    int rv;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    
    /* Enough space for any address */ //line:netp:echoserveri:sockaddrstorage
    struct sockaddr_storage clientaddr; // their_addr
    socklen_t clientlen; //sin_size
    

    fd_set master, read_fds;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);

    FD_ZERO(&master);
    FD_SET(0, &master);
    FD_SET(listenfd, &master);

    int quit = 0;
    while (1)
    {
        read_fds = master;
        Select(listenfd + 1, &read_fds, NULL, NULL, NULL);

        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        Close(connfd);
    }
    exit(0);
}
/* $end echoserverimain */