#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <local_port> <msg_size>\n", argv[0]);
        exit(1);
    }

    int msg_size = atoi(argv[2]);

    int loc_sockfd, loc_newsockfd, tamanho;
    struct sockaddr_in loc_addr;

    // Allocate a buffer that matches client message size
    char *buffer = malloc(msg_size);
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    loc_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (loc_sockfd < 0) {
        perror("socket");
        exit(1);
    }

    loc_addr.sin_family = AF_INET;
    loc_addr.sin_addr.s_addr = INADDR_ANY;
    loc_addr.sin_port = htons(atoi(argv[1]));
    bzero(&(loc_addr.sin_zero), 8);

    if (bind(loc_sockfd, (struct sockaddr *)&loc_addr, sizeof(struct sockaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(loc_sockfd, 5);
    printf("> Waiting for connection...\n");

    tamanho = sizeof(struct sockaddr_in);
    loc_newsockfd = accept(loc_sockfd, (struct sockaddr *)&loc_addr, &tamanho);
    printf("> Client connected.\n");

    while (1) {
        int n = recv(loc_newsockfd, buffer, msg_size, 0);
        if (n <= 0) break;  // client closed connection or error
        //printf("Received %d bytes\n", n);
    }

    printf("> Connection closed.\n");
    close(loc_newsockfd);
    close(loc_sockfd);
    free(buffer);
    return 0;
}

