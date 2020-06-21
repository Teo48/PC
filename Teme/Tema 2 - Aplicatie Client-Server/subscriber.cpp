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

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Usage: ./subscriber <ID> <IP_SERVER> <PORT_SERVER>.\n");
		exit(EXIT_FAILURE);
	}

	if (strlen(argv[1]) > 10) {
		fprintf(stderr, "Subscriber ID must not be longer than 10 characters.\n");
		exit(EXIT_SUCCESS);
	}

	char *name = (char *) calloc(MAX_CLIENT_ID_SIZE, sizeof(char));
	
	MEM_ALLOC(name, name == NULL, "Couldn't allocate memory! Exiting...\n");
	memcpy(name, argv[1], strlen(argv[1]));
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char *buffer = (char *) calloc(BUFLEN, sizeof(char));
	MEM_ALLOC(buffer, buffer == NULL, "Couldn't allocate memory! Exiting...\n");
	// Se completeaza campurile pentru clientul TCP corespunzator conexiunii la
	// server
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ASSERT(buffer, name, serv_addr.sin_port < 1024, "Wrong port number!");
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	ASSERT(buffer, name, ret == 0, "inet_aton");

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	
	// Se creeaza socketul pentru comunicarea cu serverul.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(buffer, name, sockfd == 0, "Couldn't create socket!");
	// Se seteaza file descriptorii pentru intrarea standard si server
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;
	int flag = 1;
	// Se dezactiveaza algoritmul lui Neagle
	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	ASSERT(buffer, name, ret < 0, "Couldn't connect!");
	// Trimite numele clientului
	n = send(sockfd, name, MAX_CLIENT_ID_SIZE, 0);
	ASSERT(buffer, name, n < 0, "Client ID!");

	while (1) {
		tmp_fds = read_fds;

  		ret = select(fdmax + 1, &tmp_fds, nullptr, nullptr, nullptr);
  		ASSERT(buffer, name, ret < 0, "Unable to select!");

  		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			// Daca s-a returnat -1 inseamna ca s-a primit comanda exit
			// iar executia subscriberului se termina.
			if (send_command(sockfd, buffer, name) == -1) {
				break;
			}
  		}

		if (FD_ISSET(sockfd, &tmp_fds)) {
  			uint16_t len;
  			uint16_t recv_size = 0;

			recv_size = recv(sockfd, (uint16_t *)&len, sizeof(uint16_t), 0);
			// Daca nu mai primeste date de la server inseamna ca serverul
			// a trimis comanda exit
  			if (recv_size == 0) {
  				break;
  			}
			
			recv_size = 0;
			uint16_t curr_read;
			len = ntohs(len);
			// Recptioneaza mesajul.
  			memset(buffer, 0, BUFLEN);
  			for (recv_size = 0; recv_size != len; recv_size += curr_read) {
  				curr_read = read(sockfd, buffer, len - recv_size);
  				ASSERT(buffer, name, curr_read <= 0, "No message!");
  			}

  			parse_udp_client_message(buffer);
  		}
	}

	free(buffer);
	free(name);
	close(sockfd);
	return 0;
}
