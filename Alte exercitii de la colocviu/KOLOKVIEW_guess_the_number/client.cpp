#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include "helpers.h"

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd = 0,ret, n, first_msg = 0;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr;

    if(argc != 4)
    {
        printf("\n Usage: %s <ip> <port> <num>\n", argv[0]);
        return 1;
    }


    fd_set read_fds;
    fd_set tmp_fds;

    int fdmax;

    FD_ZERO(&tmp_fds);
    FD_ZERO(&read_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
    FD_SET(0, &read_fds);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    ret = inet_aton(argv[1], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton");

    ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "connect");


    while (1) {
        memset(buffer, 0, BUFLEN);
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        if (FD_ISSET(0, &tmp_fds)) {  // citeste de la tastatura
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            } else {
                buffer[strlen(buffer) - 1] = '\0';
                n = send(sockfd, buffer, strlen(buffer), 0);
                DIE(n < 0, "send");
            }
        } else {
            memset(buffer, 0, BUFLEN);
            int recv_bytes = recv(sockfd, buffer, BUFLEN, 0);
            if (strcmp(buffer, "Numarul introdus este prea mic! Introduceti unul mai mare!\n") == 0) {
                printf("Numarul introdus este prea mare! Introduceti unul mai mic!\n");
                memset(buffer, 0, BUFLEN);
                fgets(buffer, BUFLEN - 1, stdin);
                buffer[strlen(buffer) - 1] = '\0';
                n = send(sockfd, buffer, strlen(buffer), 0);
                DIE(n < 0, "send");
                continue;
            } else if (strcmp(buffer, "Numarul introdus este prea mare! Introduceti unul mai mic!\n") == 0) {
                printf("Numarul introdus este prea mic! Introduceti unul mai mare!\n");
                memset(buffer, 0, BUFLEN);
                fgets(buffer, BUFLEN - 1, stdin);
                buffer[strlen(buffer) - 1] = '\0';
                n = send(sockfd, buffer, strlen(buffer), 0);
                DIE(n < 0, "send");
                continue;
            } else {
                printf("Ai castigat!\n");
                memset(buffer, 0, BUFLEN);
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
