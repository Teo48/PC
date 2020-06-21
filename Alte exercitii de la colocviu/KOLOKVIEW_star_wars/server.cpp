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

// Primul element din pereche e numele, al doilea e votul.
typedef std::pair<std::string, int> id_vote_pair;
static std::map<int, id_vote_pair> votes;
// Primul element din pereche e id-ul filimului, al doilea daca clientul a votat sau nu.
static std::map<std::pair<int, int>, bool> voted_for;

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
					printf("S-a conectat un nou client, pe socketul %d\n", newsockfd);
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
                        continue;
                    }

					if (strncmp(buffer, "ADD", 3) == 0) {
						std::string aux(buffer + 4);
						votes.insert(std::make_pair(cnt, std::make_pair(aux, 0)));
						voted_for.insert(std::make_pair(std::make_pair(i, cnt), false));
						++cnt;
						memset(buffer, 0, sizeof(buffer));
						std::string resp("Filmul a fost adaugat!\n");
						memcpy(buffer, resp.data(), resp.length());
						send(i, buffer, strlen(buffer), 0);
					} else if (strncmp(buffer, "LIST", 4) == 0) {
						std::string aux("");

						for (auto it : votes) {
							aux += std::to_string(it.first);
							aux += ". ";
							aux += it.second.first;
							aux += " - ";
							aux += std::to_string(it.second.second);
							aux += " voturi\n";
						}

						memset(buffer, 0, sizeof(buffer));
						memcpy(buffer, aux.data(), aux.length());
						send(i, buffer, strlen(buffer), 0);
					} else if (strncmp(buffer, "VOTE", 4) == 0) {
						std::string aux(buffer + 5);
						int id = atoi(aux.c_str());
						if (voted_for[{i, id}] == true) {
							std::string error("Ai votat deja acest film!\n");
							send(i, error.data(), error.length(), 0);
							error.clear();
						} else if (voted_for[{i, id}] == false) {
						voted_for[{i, id}] = true;
						++votes[id].second;
						}
					}
                }
            }
        }
    }
	close(sockfd);

	return 0;
}