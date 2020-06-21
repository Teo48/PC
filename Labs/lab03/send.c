#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char *argv[]) {
	msg t;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);

	if (argc < 2) {
		fprintf(stderr, "Argumentele Gigele, argumentele!");
		exit(EXIT_FAILURE);
	}

	int BDP = atoi(argv[1]);
	int window_size = (BDP * 1000) / (MSGSIZE * 8);

	printf("Window size: %d\n", window_size);

	char buffer[MSGSIZE] = "PA, PP, PC. In that order!";
	buffer[MSGSIZE - 1] = '\0';
	for (int i = 0 ; i < window_size ; ++i) {
		memset(&t, 0, sizeof(msg));		
		memcpy(t.payload, buffer, MSGSIZE);
		t.len = MSGSIZE;

		for (int i = 0 ; i < MSGSIZE ; ++i) {
				for (int j = 0 ; j < 8 ; ++j) {
					t.checksum ^= (1 << j) & t.payload[i];
				}
			}

		if (send_message(&t) < 0) {
			perror("[SENDER] Send error.\n");
			exit(EXIT_FAILURE);
		}
	}

	char next_buffer[MSGSIZE] = "lorem ipsum dolor sit amet, consectetur adipiscing elit. duis laoreet urna non.";
	next_buffer[MSGSIZE - 1] = '\0';
	
	for (int i = window_size ; i < COUNT ; ++i) {
		memset(&t, 0, sizeof(t));
		if (recv_message(&t) < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			exit(EXIT_FAILURE);
		} else {
			printf("[SENDER] Received ACK: %s\n", t.payload);
		}

		memcpy(t.payload, next_buffer, MSGSIZE);
		t.len = MSGSIZE;
		memset(&t.checksum, 0, sizeof(int));

		for (int i = 0 ; i < MSGSIZE ; ++i) {
				for (int j = 0 ; j < 8 ; ++j) {
					t.checksum ^= (1 << j) & t.payload[i];
				}
			}

		if (send_message(&t) < 0) {
			exit(EXIT_FAILURE);
			perror("[SENDER] Send error. Exiting.\n");
		}
	}

	for (int i = 0 ; i < window_size ; ++i) {
		if (recv_message(&t) < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			exit(EXIT_FAILURE);
		} else {
			printf("[SENDER] Received ACK: %s\n", t.payload);
		}
	}

	printf("[SENDER] Job done, all sent.\n");
	return 0;
}
