#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include "utils.h"
#include "subscriber_utils.h"

// Trimite o comanda la server
int send_command(int sockfd, char *buffer, char *name) {
	// Citeste tipul comenzii
	TCP_CLIENT_REQUEST req;
	memset(&req, 0, sizeof(TCP_CLIENT_REQUEST));
	fscanf(stdin, "%s", req.request_type);
	
	// Daca nu este o comanda de tip subscribe sau unsubscribe se verifica
	// daca nu cumva este una de tip exit, in caz contrar se semnaleza eroare.
	if (strcmp(req.request_type, "subscribe") && strcmp(req.request_type, "unsubscribe")) {
		if (strncmp(req.request_type, "exit", 4) == 0) {
			return -1;
		}

		fprintf(stderr, "Invalid command %s\n", req.request_type);
		fprintf(stderr, "Valid commands are: subscribe and unsubscribe\n");
		
		return 0;
	}

	// Citeste numele topicului
	fscanf(stdin, "%s", req.topic);
	
	// Daca cererea este de tip subscribe se citeste SF
	// si se verifica daca este 0 sau 1.
	if (strcmp(req.request_type, "subscribe") == 0) {
		fscanf(stdin, "%s", &req.SF);
		if (req.SF != 48 && req.SF != 49) {
			fprintf(stderr, "SF must be either 0 or 1. Please, reintroduce the command!\n");
			return 0;
		}
	}

	// Se trimite mesajul la server si se afiseaza in consola
	// ca s-a abonat sau dezabonat de la topic.
	int n = send(sockfd, &req, sizeof(TCP_CLIENT_REQUEST), 0);
	ASSERT(buffer, name, n < 0, "Send");

	fprintf(stdout, "%sd %s\n", req.request_type, req.topic);

	return 1;
}

// Extrage din buffer 5 bytes si reconstruieste int-ul conform
// specificatiilor din enuntul temei
uint32_t decode_int(char *buffer, uint16_t cursor) {
	uint32_t n;
	uint32_t n1 = buffer[2 + cursor] >= 0 ? buffer[2 + cursor] : buffer[2 + cursor] + 256;
	uint32_t n2 = buffer[3 + cursor] >= 0 ? buffer[3 + cursor] : buffer[3 + cursor] + 256;
	uint32_t n3 = buffer[4 + cursor] >= 0 ? buffer[4 + cursor] : buffer[5 + cursor] + 256;
	uint32_t n4 = buffer[5 + cursor] >= 0 ? buffer[5 + cursor] : buffer[5 + cursor] + 256;
	n = n1 << 24 | n2 << 16 | n3 << 8 | n4;

	if (buffer[1 + cursor] == 1) {
		n *= -1;
	}

	return n;
}

// Extrage din buffer 3 bytes si reconstruieste short-ul conform
// specificatiilor din enuntul temei
uint16_t decode_short_real(char *buffer, uint16_t cursor) {
	uint16_t n;
	uint16_t n1 = buffer[1 + cursor] >= 0 ? buffer[1 + cursor] : buffer[1 + cursor] + 256;
	uint16_t n2 = buffer[2 + cursor] >= 0 ? buffer[2 + cursor] : buffer[2 + cursor] + 256;
	n = n1 << 8 | n2;

	return n;
} 

// Extrage din buffer 6 bytes si reconstruieste float-ul conform
// specificatiilor din enuntul temei
double decode_float(char *buffer, uint16_t cursor) {
	uint32_t n;
	uint32_t n1 = buffer[2 + cursor] >= 0 ? buffer[2 + cursor] : buffer[2 + cursor] + 256;
	uint32_t n2 = buffer[3 + cursor] >= 0 ? buffer[3 + cursor] : buffer[3 + cursor] + 256;
	uint32_t n3 = buffer[4 + cursor] >= 0 ? buffer[4 + cursor] : buffer[5 + cursor] + 256;
	uint32_t n4 = buffer[5 + cursor] >= 0 ? buffer[5 + cursor] : buffer[5 + cursor] + 256;
	n = n1 << 24 | n2 << 16 | n3 << 8 | n4;
	 		
	double fn = n;
	for (uint8_t exp = buffer[2 + cursor + sizeof(uint32_t)]; exp ; fn*= 0.1, --exp);
	
	if (buffer[1 + cursor] == 1) {
		fn *= -1;
	}
	return fn;
}

// Se parseaza mesajul de la clientul udp in functie de tipul acestuia
void parse_udp_client_message(char *buffer) {
	char client_ip[INET_ADDRSTRLEN];
	uint16_t len, client_port;
	sprintf(client_ip, "%s", buffer + 1);
	len = strlen(buffer + 1) + 1;
	memcpy(&client_port, buffer + 1 + len, sizeof(uint16_t));
	client_port = ntohs(client_port);
	len += sizeof(uint16_t);
	char message_type[MAX_REQUEST_SIZE];
	sprintf(message_type, "%s", buffer + 1 + len);
	len += strlen(buffer + 1 + len) + 1;
	fprintf(stdout, "%s:%u - %s - ", client_ip, client_port, message_type);
	
	switch(buffer[0]) {
	 	case INT : {
			fprintf(stdout, "INT - %d\n", decode_int(buffer, len));
			break;
		}
		case SHORT_REAL : {
			fprintf(stdout, "SHORT_REAL - %.02f\n", decode_short_real(buffer, len) / 100.0);
			break;
		}
		case FLOAT : {
			fprintf(stdout, "FLOAT - %f\n", decode_float(buffer, len));
			break;
		}
		case STRING : {
			fprintf(stdout, "STRING - %s\n", buffer + 1 + len);
			break;
		}
		default : {
			fprintf(stdout, "Corrupted message\n");
		}
	}
}