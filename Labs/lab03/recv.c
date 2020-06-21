#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char *argv[]) {
	msg r, t;
	int counter = 0;
	init(HOST, PORT);
	
	for (int i = 0 ; i < COUNT ; ++i) {
		memset(&r, 0, sizeof(r));
		memset(&t, 0, sizeof(msg));
		
		if (recv_message(&r) < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			exit(EXIT_FAILURE);
		}

		memcpy(t.payload, r.payload, MSGSIZE);

		if (r.checksum != 0) {
			for (int i = 0 ; i < MSGSIZE ; ++i) {
				for (int j = 0 ; j < 8 ; ++j) {
					t.checksum ^= (1 << j) & t.payload[i];
				}
			}
		} else {
			t.checksum = 0;
		}

		if (t.checksum != r.checksum) {
			printf("[REICEVER] Receive error. Checksum is different.\n");
			++counter;
		}

		if (send_message(&r) < 0) {
			perror("[RECEIVER] SEND ACK error. Exiting.\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("[RECEIVER] Finished receiving..\n");
	printf("S-au corupt %d mesaje \n", counter);
	return 0;
}
