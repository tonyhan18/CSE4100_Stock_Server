/* 
 * echoserveri.c - An iterative echo server 
 */
/* $begin echoserverimain */
#include "csapp.h"

#define BST_OK 0x00000000
#define BST_MALLOC_FAIL 0xff000001
#define BST_KEY_ALREADY_EXIST 0xff000002
#define BST_KEY_NOT_EXIST 0xff000003
#define BST_NODE_IS_NULL 0xff000004

typedef struct s_item
{
    int ID;
    int readcnt;
    int price;
    struct s_item *left_stock;
    struct s_item *right_stock;
    sem_t mutex;
} item;

const char *LIMITER = " ";

void echo(int connfd);

int insertNode(item **root, int ID, int readcnt, int price)
{
    item *p, *t;
    item *n;

    t = *root;
    p = NULL;

    while (t != NULL)
    {
        if (ID == t->ID)
        {
            return (BST_KEY_ALREADY_EXIST);
        }
        p = t;

        if (ID < t->ID)
        {
            t = t->left_stock;
        }
        else
        {
            t = t->right_stock;
        }
    }

    n = (item *)Malloc(sizeof(item));
    if (n == NULL)
        return (BST_MALLOC_FAIL);

    n->ID = ID;
    n->readcnt = readcnt;
    n->price = price;
    n->left_stock = NULL;
    n->right_stock = NULL;
    Sem_init(&n->mutex, 0, 1);

    if (p != NULL)
    {
        if (ID < p->ID)
            p->left_stock = n;
        else
            p->right_stock = n;
    }
    else
        *root = n;

    return BST_OK;
}

item *searchNode(item *node, int ID)
{
    while (node != NULL)
    {
        if (ID == node->ID)
        {
            return node;
        }
        else if (ID < node->ID)
        {
            node = node->left_stock;
        }
        else
        {
            node = node->right_stock;
        }
    }
    return (NULL);
}

void preOrder(item *root)
{
    if (root)
    {
        printf("%d %d %d\n", root->ID, root->readcnt, root->price);
        preOrder(root->left_stock);
        preOrder(root->right_stock);
    }
}

void tablePreOrder(item *root, FILE *fp)
{
    if (root)
    {
        char tmp[MAXLINE];
        P(&root->mutex);
        sprintf(tmp, "%d %d %d\n", root->ID, root->readcnt, root->price);
        Fputs(tmp, fp);
        tablePreOrder(root->left_stock, fp);
        tablePreOrder(root->right_stock, fp);
        V(&root->mutex);
    }
}

void tablePreOrder2(item *root, char *ans)
{
    if (root)
    {
        char tmp[MAXLINE];
        P(&root->mutex);
        sprintf(tmp, "%d %d %d\n", root->ID, root->readcnt, root->price);
        strcat(ans, tmp);
        tablePreOrder2(root->left_stock, ans);
        tablePreOrder2(root->right_stock, ans);
        V(&root->mutex);
    }
}

item *tableInit(void)
{
    FILE *fp = Fopen("stock.txt", "r");
    char *LIMITER = " ";

    if (!fp)
    {
        perror("File does not exists");
    }

    char s[MAXBUF];
    //read all line by line
    item *root = NULL;
    while (Fgets(s, MAXBUF, fp) != NULL)
    {
        s[strlen(s) - 1] = '\0';
        char *token = strtok(s, LIMITER);
        int ID = atoi(token);
        token = strtok(NULL, LIMITER);
        int readcnt = atoi(token);
        token = strtok(NULL, LIMITER);
        int price = atoi(token);
        insertNode(&root, ID, readcnt, price);
    }
    return (root);
}

int tableSaved(item *root)
{
    FILE *fp = Fopen("stock.txt", "w");
    if (!fp)
        return (0);
    tablePreOrder(root, fp);
    Fclose(fp);
    return (1);
}

int buyStock(item *root, int ID, int readcnt)
{
    item *node;
    if ((node = searchNode(root, ID)) == NULL)
    {
        return 0;
    }

    P(&node->mutex);
    if (node->readcnt >= readcnt)
    {
        node->readcnt -= readcnt;
        V(&node->mutex);
        return 1;
    }
    else
    {
        V(&node->mutex);
        return 0;
    }
}

int sellStock(item *root, int ID, int readcnt)
{
    item *node;
    if ((node = searchNode(root, ID)) == NULL)
    {
        return 0;
    }

    P(&node->mutex);
    node->readcnt += readcnt;
    V(&node->mutex);
    return 1;
}

int main(int argc, char **argv)
{
    int listenfd; //sockfd -> save the server socket
    int connfd;   //new_fd
    int numbytes;
    char buf[MAXBUF];
    //struct addrinfo hints, *servinfo;
    //int rv;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    struct sockaddr_storage clientaddr; // their_addr
    socklen_t clientlen;                //sin_size

    fd_set master, read_fds;
    int fdmax; // 가장 큰 소켓값, 소켓의 범위

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = Open_listenfd(argv[1]);
    item *root = tableInit();

    FD_SET(listenfd, &master);
    fdmax = listenfd;

    while (1)
    {
        //preOrder(root); //print graph
        read_fds = master;
        Select(fdmax + 1, &read_fds, NULL, NULL, NULL);

        for (int i = 0; i <= fdmax; i++)
        {
            //check out if msg is coming : 1
            if (FD_ISSET(i, &read_fds))
            {
                if (i == listenfd)
                {
                    clientlen = sizeof clientaddr;
                    connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

                    if (connfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(connfd, &master);
                        if (connfd > fdmax)
                        {
                            fdmax = connfd;
                        }
                        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
                        printf("Connected to (%s, %s)\n", client_hostname, client_port);
                    }
                } // End of receive msg from listenfd
                else
                {
                    rio_t rio;
                    char ans[MAXLINE] = "";

                    Rio_readinitb(&rio, i);
                    if ((numbytes = Rio_readlineb(&rio, buf, MAXLINE)) <= 0)
                    {
                        strcpy(ans, "exit\n");
                        Rio_writen(i, ans, strlen(ans));
                        Close(i);
                        FD_CLR(i, &master);
                        tableSaved(root);
                    } // end of doesn't recv items or end of socket
                    else
                    {
                        buf[numbytes] = '\0';
                        printf("server received %d bytes\n", numbytes);
                        char tmp[MAXLINE];
                        strcpy(tmp, buf);

                        char *token = strtok(tmp, LIMITER);
                        //printf(">>> token : %s | buf : %s <<<\n",token, buf);
                        if (strncmp(token, "show", 4) == 0)
                        {
                            tablePreOrder2(root, ans);
                            ans[strlen(ans)] = '\0';
                            Rio_writen(i, ans, strlen(ans));
                        }
                        else if (strncmp(token, "buy", 3) == 0)
                        {
                            int ID = atoi(strtok(NULL, LIMITER));
                            int readcnt = atoi(strtok(NULL, LIMITER));
                            if (buyStock(root, ID, readcnt))
                            {
                                strcpy(buf, "[buy] success\n");
                            }
                            else
                            {
                                strcpy(buf, "Not enough left stock\n");
                            }
                            Rio_writen(i, buf, strlen(buf));
                        }
                        else if (strncmp(token, "sell", 4) == 0)
                        {
                            int ID = atoi(strtok(NULL, LIMITER));
                            int readcnt = atoi(strtok(NULL, LIMITER));
                            if (sellStock(root, ID, readcnt))
                            {
                                strcpy(buf, "[Sell] success\n");
                                Rio_writen(i, buf, strlen(buf));
                            }
                        }
                        else if (strncmp(token, "exit", 4) == 0)
                        {
                            printf("exit\n");
                            strcpy(ans, "exit\n");
                            Rio_writen(i, ans, strlen(ans));
                            tableSaved(root);
                            Close(i);
                            FD_CLR(i, &master);
                            continue;
                        }
                        else if (strncmp(token, "disc", 4) == 0)
                        {
                            tableSaved(root);
                            continue;
                        }
                        tableSaved(root);
                        strcpy(ans, "EOF\n");
                        Rio_writen(i, ans, strlen(ans));
                    } // end of recv items
                }     // End of receive msg
            }
        }
    }
    exit(0);
}
/* $end echoserverimain */