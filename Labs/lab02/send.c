#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

void send_info(const char *file_name) {
	msg t;
	memset(t.payload, 0, MAX_LEN);
	int size, block_size;
	memcpy(t.payload, file_name, MAX_LEN);

	t.len = strlen(t.payload) + 1;

	send_message(&t);

	if (recv_message(&t) < 0) {
		perror("Receive error");
	} else {
		// printf("[send] Got reply with payload: %s\n", t.payload);
	}

	printf("%s\n", t.payload);

	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Eroare la deschiderea fisierului!\n");
		exit(EXIT_FAILURE);
	}

	size = lseek(fd, 0, SEEK_END);
	sprintf(t.payload, "%d", size);
	t.len = strlen(t.payload) + 1;

	send_message(&t);

	if (recv_message(&t) < 0) {
		perror("Receive error");
	} else {
		// printf("[send] Got reply with payload: %s\n", t.payload);
	}

	lseek(fd, 0, SEEK_SET);

	while ((block_size = read(fd, t.payload, MAX_LEN - 1))) {
		if (block_size < 0) {
			fprintf(stderr, "Nu s-a putut citi din fisier!\n");
			exit(EXIT_FAILURE);
		} else {
			t.len = block_size;
			send_message(&t);

			if (recv_message(&t) < 0) {
				perror("Receive error");
			} else {
				// printf("[send] Got reply with payload: ACK<%s> \n", t.payload);
			}

			memset(t.payload, 0, MAX_LEN);
		}
	}

	close(fd);
}
	
int main(int argc,char** argv){
	init(HOST,PORT);
	for (int i = 1 ; i < argc ; ++i) {
		send_info(argv[i]);
		printf("--------------------------------------------------------------------------\n");
	}
	
  	return 0;
}
