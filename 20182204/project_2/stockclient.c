/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        Rio_writen(clientfd, buf, strlen(buf));

        //바뀐 부분
        int n;
        while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
        {
            printf("server : %s\n",buf);
            if(strncmp(buf, "EOF",3) == 0)
                break;
            if(strncmp(buf,"exit",4) == 0)
                break;
            Rio_writen(STDOUT_FILENO, buf, n);
        }
    }
    strcpy(buf, "disc\n");
	Rio_writen(clientfd, buf, strlen(buf));
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
