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

void tablePreOrder(item *root, int i)
{
    if (root)
    {
        char ans[MAXLINE];
        sprintf(ans, "%d %d %d\n", root->ID, root->readcnt, root->price);
        Rio_writen(i,ans,strlen(ans));
        printf("ans : %s\n",ans);
        tablePreOrder(root->left_stock,i);
        tablePreOrder(root->right_stock,i);
    }
}

void tablePreOrder2(item *root, char *ans)
{
    if (root)
    {
        char tmp[MAXLINE];
        sprintf(tmp, "%d %d %d\n", root->ID, root->readcnt, root->price);
        strcat(ans,tmp);
        tablePreOrder2(root->left_stock, ans);
        tablePreOrder2(root->right_stock, ans);
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

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
	clientlen = sizeof(struct sockaddr_storage); 
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
	echo(connfd);
	Close(connfd);
    }
    exit(0);
}
/* $end echoserverimain */
