#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 1024
#define PORT 8080
#define SA struct sockaddr

int sockfd;

void *recv_func(void *arg) {
    char buff[MAX];
    while (1) {
        bzero(buff, MAX);
        int n = read(sockfd, buff, sizeof(buff));
        if (n <= 0) {
            printf("Server disconnected.\n");
            close(sockfd);
            exit(0);
        }
        printf("\nServer: %s", buff);
        fflush(stdout);
    }
    return NULL;
}

void *send_func(void *arg) {
    char buff[MAX];
    while (1) {
        bzero(buff, MAX);
        fgets(buff, MAX, stdin);
        write(sockfd, buff, strlen(buff));

        if (strncmp(buff, "exit", 4) == 0) {
            printf("Closing connection...\n");
            close(sockfd);
            exit(0);
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket created..\n");

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.141.185"); // change to server IP

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed...\n");
        exit(0);
    }
    printf("Connected to server.\n");

    pthread_t recv_thread, send_thread;
    pthread_create(&recv_thread, NULL, recv_func, NULL);
    pthread_create(&send_thread, NULL, send_func, NULL);

    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    close(sockfd);
    return 0;
}
