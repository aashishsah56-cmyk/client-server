#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 1024
#define PORT 8080
#define SA struct sockaddr

// Thread for receiving messages from a client
void *recv_func(void *arg) {
    int connfd = *(int *)arg;
    char buff[MAX];

    while (1) {
        bzero(buff, MAX);
        int n = read(connfd, buff, sizeof(buff));
        if (n <= 0) {
            printf("Client disconnected.\n");
            close(connfd);
            pthread_exit(NULL);
        }
        printf("\nClient: %s", buff);
        fflush(stdout);
    }
    return NULL;
}

// Thread for sending messages to a client
void *send_func(void *arg) {
    int connfd = *(int *)arg;
    char buff[MAX];

    while (1) {
        bzero(buff, MAX);
        fgets(buff, MAX, stdin);
        write(connfd, buff, strlen(buff));

        if (strncmp(buff, "exit", 4) == 0) {
            printf("Closing connection...\n");
            close(connfd);
            pthread_exit(NULL);
        }
    }
    return NULL;
}

void *client_handler(void *arg) {
    int connfd = *(int *)arg;
    free(arg);

    pthread_t recv_thread, send_thread;
    pthread_create(&recv_thread, NULL, recv_func, &connfd);
    pthread_create(&send_thread, NULL, send_func, &connfd);

    pthread_join(recv_thread, NULL);
    pthread_cancel(send_thread);

    return NULL;
}

int main() {
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket created..\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Bind failed...\n");
        exit(0);
    }
    printf("Bind successful..\n");

    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    printf("Server listening..\n");

    len = sizeof(cli);
    while (1) {
        connfd = accept(sockfd, (SA*)&cli, (socklen_t *)&len);
        if (connfd < 0) {
            printf("Accept failed...\n");
            continue;
        }
        printf("Client connected!\n");

        int *new_sock = malloc(sizeof(int));
        *new_sock = connfd;
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, new_sock);
        pthread_detach(tid);
    }

    close(sockfd);
    return 0;
}