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

static std::map<int, std::string> client_name_map;
static std::pair<int, int> count_proc = std::make_pair(0, 0);

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

std::vector<std::string> files{"gigel.in", "date.in", "test.in"};

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

					printf("Noua conexiune de la %s, port %d, socket client %d\n",
							inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
					client_name_map.insert(std::make_pair(newsockfd, "Nelogat"));
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

					if (client_name_map[i] == "Nelogat") {
						if (strncmp(buffer, "auth", 4) == 0) {
							client_name_map[i] = std::string(buffer + 5);
						}
						memset(buffer, 0, sizeof(buffer));
						std::string aux("");
						aux += "V-ati logat cu succes!\n";
						memcpy(buffer, aux.data(), aux.length());
						send(i, buffer, strlen(buffer), 0);
						aux.clear();
					} else {
						if (strncmp(buffer, "date_time", 9) == 0) {
							std::string aux = currentDateTime();
							aux += "\n";
							memset(buffer, 0, BUFLEN);
							memcpy(buffer, aux.data(), aux.length());
							send(i, buffer, strlen(buffer), 0);
							aux.clear();
							++count_proc.first;
						} else if (strncmp(buffer, "len", 3) == 0) {
							++count_proc.second;
							std::string file_name = std::string(buffer + 4);
							memset(buffer, 0, BUFLEN);
							std::string aux("");
							bool found = false;
							for (auto it : files) {
								if (it == file_name) {
									found = true;
									int source_file = open(it.c_str(), O_RDONLY);
									off_t start_file = lseek(source_file, 0, SEEK_SET);
									off_t end_file = lseek(source_file, 0, SEEK_END);
									size_t file_size = end_file - start_file;
									aux += std::to_string(file_size);
									close(source_file);
								}
							}

							aux += "\n";
							if (found == true) {
								memcpy(buffer, aux.data(), aux.length());
								send(i, buffer, strlen(buffer), 0);
								aux.clear();
							} else {
								std::string error("Nu exista fisierul!\n");
								memcpy(buffer, error.data(), error.length());
								send(i, buffer, strlen(buffer), 0);
								aux.clear();
							}
						} else if (strncmp(buffer, "stats", 5) == 0) {
							memset(buffer, 0, BUFLEN);
							if (client_name_map[i] != "administrator") {
								std::string error("Nu sunteti administrator!\n");
								memcpy(buffer, error.data(), error.length());
								send(i, buffer, strlen(buffer), 0);
								error.clear();
							} else {
								double stats_date, stats_len;
								double total = (count_proc.first + count_proc.second);
								stats_date = (double)  ((count_proc.first) * 100) / total;
								stats_len =  (double) ((count_proc.second) * 100) / total;
								std::string aux("");
								aux += "Date stats: ";
								aux += std::to_string(stats_date);
								aux += "%\n";
								aux += "Len stats: ";
								aux += std::to_string(stats_len);
								aux += "%\n";
								memcpy(buffer, aux.data(), aux.length());
								send(i, buffer, strlen(buffer), 0);
								aux.clear();
							}
						}
					}
                }
            }
        }
    }
	close(sockfd);

	return 0;
}