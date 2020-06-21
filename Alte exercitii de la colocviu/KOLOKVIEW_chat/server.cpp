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

static std::map<int, std::string> clients;
static std::map<std::string, int> clients_id;
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

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}
	
	
	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0); // se asteapta conexiuni
	DIE(sockfd < 0, "socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	FD_SET(sockfd, &read_fds);
	// Doar daca vreau ca serverul sa primeasca exit.
	FD_SET(0, &read_fds);
	fdmax = sockfd;
	int cnt = 1;
	bool transmission_ended = true;
	while (transmission_ended) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);

					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}

					// printf("Noua conexiune de la %s, port %d, socket client %d\n",
					// 		inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
					memset(buffer, 0, BUFLEN);
					n = recv(newsockfd, buffer, sizeof(buffer), 0);
                	DIE(n < 0, "recv");
					std::string aux(buffer);
					printf("S-a conectat un nou client, pe socketul %d\n", newsockfd);
					if (clients_id.find(aux) == clients_id.end()) {
						clients.insert(std::make_pair(cnt++, aux));
					} else {
						clients.insert(std::make_pair(clients_id[aux], aux));
					}
					
					aux.clear();
				} else if (i == STDIN_FILENO) {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					buffer[strlen(buffer) - 1] = 0;
					if (strncmp(buffer, "exit", 4) == 0) {
						fprintf(stdout, "Se inchide serverul!\n");
						transmission_ended = false;
						break;
					}
				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

                    if (n == 0) {
                        // conexiunea s-a inchis
                        printf("Socket-ul client %d a inchis conexiunea\n", i);
                        close(i);
                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(i, &read_fds);
						clients_id.insert(std::make_pair(clients[i - 3], i - 3));
						clients.erase(i - 3);
                        continue;
                    }

					if (strncmp(buffer, "MSG", 3) == 0) {
						
						std::string aux("");
						int index = 0;
						for (int i = 4; i < strlen(buffer) ; ++i) {
							if (buffer[i] == ' ') {
								index = i;
								break;
							}
							aux += buffer[i];
						}

						int send_to_nr_client = atoi(aux.c_str());

						aux.clear();

						std::string msg(buffer + index + 1);
						msg += "\n";
						send(send_to_nr_client + 3, msg.data(), msg.length(), 0);
					} else if (strncmp(buffer, "LIST", 4) == 0) {
						std::string client_list("");
						for (auto it : clients) {
							client_list += std::to_string(it.first);
							client_list += "-";
							client_list += it.second;
							client_list += "\n";
						}

						send(i, client_list.data(), client_list.length(), 0);
						client_list.clear();
					}
                }
            }
        }
    }
	close(sockfd);

	return 0;
}