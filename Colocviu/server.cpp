#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include <time.h>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <fcntl.h>

static int sum = 0;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{	
	srand(time(NULL));
	int sockfd, newsockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;

	fd_set read_fds;	
	fd_set tmp_fds;
	int fdmax;			

	if (argc < 2) {
		usage(argv[0]);
	}

	int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = bind(udp_sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);

	FD_SET(udp_sockfd, &read_fds);
	fdmax = std::max(sockfd, udp_sockfd);
	int cnt = 1;
	bool transmission_ended = true;
	while (transmission_ended) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					FD_SET(newsockfd, &read_fds);

					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
				} else if (i == STDIN_FILENO) {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					buffer[strlen(buffer) - 1] = 0;
					if (strncmp(buffer, "exit", 4) == 0) {
						fprintf(stdout, "Se inchide serverul!\n");
						transmission_ended = false;
						break;
					}
				} else if (i == udp_sockfd) {
					memset(buffer, 0, sizeof(buffer));
					struct sockaddr_in cli_addr;
					socklen_t clilen = sizeof(cli_addr);
	
					int n = recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &cli_addr, &clilen);
					std::string aux(buffer);
					int nr = atoi(aux.c_str());
					sum += nr;
					char *ip = strdup(inet_ntoa(cli_addr.sin_addr));
					int port = cli_addr.sin_port;
					printf("Am primit %d de la %s:%d\n", nr, ip, port);
					free(ip);
				} else {
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

                    if (n == 0) {
                        close(i);
                        FD_CLR(i, &read_fds);
                        continue;
                    }

					std::string aux("Suma este: ");
					aux += std::to_string(sum);
					aux += "\n";
					send(i, aux.data(), aux.length(), 0);
				}
            }
        }
    }
	close(sockfd);

	return 0;
}