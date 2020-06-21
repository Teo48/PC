#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

void recv_info() {
    msg r, t;   
    int block_size;
    memset(r.payload, 0, MAX_LEN);
    memset(t.payload, 0, MAX_LEN);

    if (recv_message(&r) < 0) {
        perror("Receive message");
        exit(EXIT_FAILURE);
    }

    // printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);

    strcat(r.payload, "-received");
    int fd = open(r.payload, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        fprintf(stderr, "Eroare la crearea fisierului!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(t.payload, "ACK<", 4);
    strcat(t.payload, r.payload);
    strcat(t.payload, ">");
    
    t.len = strlen(t.payload) + 1;
    send_message(&t);
    printf("[recv] ACK sent\n");


    if (recv_message(&r) < 0) {
        perror("Receive message");
        exit(EXIT_FAILURE);
    }

    // printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);

    int file_size = atoi(r.payload);
    printf("Input file size: %d\n", file_size);

    memset(t.payload, 0, MAX_LEN);
    memcpy(t.payload, "ACK<", 4);
    strcat(t.payload, r.payload);
    strcat(t.payload, ">");
    t.len = strlen(t.payload) + 1;
    send_message(&t);
    
    printf("[recv] ACK sent\n");

    while (file_size) {
        if (recv_message(&r) < 0) {
            perror("Receive message");
            exit(EXIT_FAILURE);
        }

        // printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);

        block_size = write(fd, r.payload, r.len);

        if (block_size < 0) {
            fprintf(stderr, "Nu s-a putut scrie!\n");
            exit(EXIT_FAILURE);
        }

        send_message(&r);
        file_size -= r.len;
        memset(r.payload, 0, MAX_LEN);
    }

    close(fd);
}

int main(int argc, char** argv){    
    init(HOST,PORT);
    for (int i = 1 ; i < argc ; ++i) {
       recv_info();
    }
    
    return 0;
}
