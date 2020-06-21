#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <map>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"
#include "server_utils.h"

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: ./server <PORT>\n");
		exit(EXIT_FAILURE);
	}

	char *buffer = (char *) calloc(BUFLEN, sizeof(char));
	MEM_ALLOC(buffer, buffer == NULL, "Couldn't allocate memory! Exiting...\n");
	// subscribed_clients este o mapa ce are drept cheie topicul, iar valorea
	// o alta mapa ce are drept cheie id-ul clientului, iar ca valoare fisierele
	// corespunzatoare topicurilor la care este abondat
	str_str_topic_map subscribed_clients;
	// clients_ids este o mapa ce ascociaza id-ului unui client, socket-ul
	// acestuia.
	str_int_map clients_ids;
	// clients_sockets este o mapa ce asocieaza socket-ul cu id-ul clientului.
	int_str_map clients_sockets;
	// clients_ips este o mapa ce asociaza ip-ul clientuli cu port-ul.
	int_port_map clinets_ips;

	// Se creeaza socket-ul tcp.
	int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	MEM_ALLOC(buffer, tcp_sockfd < 0, "TCP SOCKET\n");

	// Se creeaza socket-ul UDP
	int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	MEM_ALLOC(buffer, udp_sockfd < 0, "UDP SOCKET\n");

	// Se completeaza informatiile despre socketul UDP si cel pasiv TCP
	int portno = atoi(argv[1]);
	MEM_ALLOC(buffer, portno == 0 || portno < 1024, "Wrong port number!\n");
	struct sockaddr_in serv_addr, cli_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	MEM_ALLOC(buffer, serv_addr.sin_port < 0, "HTONS!\n");
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// Se dezactiveaza algoritmul lui Neagle
	int flag = 1;
	int ret = setsockopt(tcp_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
	MEM_ALLOC(buffer, ret < 0, "Couldn't deactivate Neagle!\n");
	ret = bind(tcp_sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	MEM_ALLOC(buffer, ret < 0, "TCP Bind!\n");
	ret = bind(udp_sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	MEM_ALLOC(buffer, ret < 0, "UDP Bind!\n");
	
	ret = listen(tcp_sockfd, MAX_CLIENTS);
	MEM_ALLOC(buffer, ret < 0, "Listen!\n");

	fd_set read_fds;
	fd_set tmp_fds;

	// Se seteaza descriptorii socketilor creati
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(tcp_sockfd, &read_fds);
	FD_SET(udp_sockfd, &read_fds);

	int fdmax = std::max(tcp_sockfd, udp_sockfd);
	bool transmission_ended = false;
	int n;
	// Serverul transmite pana cand se primeste comanda exit, iar
	// transmission_ended devine true.
	while (transmission_ended == false) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, nullptr, nullptr, nullptr);
		MEM_ALLOC(buffer, ret < 0, "Unable to select!\n");

		for (int i = 0; i <= fdmax; ++i) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == STDIN_FILENO) {
					// S-a primit o comanda de la STDIN
					// Se verifica daca este exit sau nu. Daca da
					// toti clientii conectati sunt anuntati si la randul
					// lor se inchid.
					// Atlfel, clientul este notificat ca serverul primeste
					// doar comanda exit
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);
					buffer[strlen(buffer) - 1] = 0;
					if (strncmp(buffer, "exit", 4)) {
						fprintf(stdout, "Invalid command. The only valid command is exit!\n");
					} else {
						for (int_str_map::iterator it = clients_sockets.begin() ;
							it != clients_sockets. end() ; ++it) {
								send(it->first, nullptr, 0, 0);
								close(it->first);
							}
						transmission_ended = true;
						break;
					}
				} else if (i == tcp_sockfd) {
					// S-a primit un mesaj de la un client TCP
					socklen_t clilen = sizeof(cli_addr);
					int newsockfd = accept(tcp_sockfd, (struct sockaddr *) &cli_addr, &clilen);
					MEM_ALLOC(buffer, ret < 0, "TCP accept\n");
					FD_SET(newsockfd, &read_fds);
					fdmax = std::max(newsockfd, fdmax);
					// Se creeaza o noua intrare pentru client
					clients_sockets.insert(std::make_pair(newsockfd, "Unassigned"));
					// Se retine ip-ul si port-ul
					clinets_ips.insert(std::make_pair(newsockfd, ip_port(
							ntohs(cli_addr.sin_port), std::string(inet_ntoa(cli_addr.sin_addr)))));
				} else if (i == udp_sockfd) {
					// S-a primit mesaj de la un client UDP si acesta trebuie parsat
					parse_message(subscribed_clients, clients_ids, i);
				}  else {
					// Se primeste id-ul unui client sau cererea acestuia
					uint16_t recv_size = 0;
					if (clients_sockets[i] == "Unassigned") {
						recv_size = MAX_CLIENT_ID_SIZE;
					} else {
						recv_size = sizeof(TCP_CLIENT_REQUEST);
					}
					memset(buffer, 0, BUFLEN);

					for (uint16_t len = 0 ; len != recv_size ; len += n) {
						n = recv(i, buffer + len, recv_size - len, 0);
						MEM_ALLOC(buffer, n < 0, "recv\n");
						if (n == 0) {
							break;
						}
					}
					
					
					if (n == 0) {
						// Daca clientul se deconecteaza, i se sterg intrarile active
						// si serverul notifica acest lucru la consola.
						if (clients_sockets.find(i) != clients_sockets.end()) {
							clients_ids[clients_sockets[i]] = -1;
							printf("Client %s disconnected\n", clients_sockets[i].c_str());
							clients_sockets.erase(i);
							clinets_ips.erase(i);
						}
						close(i);
						FD_CLR(i, &read_fds);
					} else {
						// Daca avem un client nou, ii cream o intrare.
						if (clients_sockets[i] == "Unassigned") {
							authenticate_client(subscribed_clients, clients_sockets,
								clients_ids, std::string(buffer), i);
							fprintf(stdout, "New client %s connected from %s:%u\n",
								clients_sockets[i].c_str(), clinets_ips[i].ip.c_str(), clinets_ips[i].port);
							continue;
						}

						switch (tcp_client_command(subscribed_clients,  clients_sockets, buffer, i)) {
							case -1:
								fprintf(stderr, "Client already unsubscribed!\n");
								break;
							case 1:
								fprintf(stderr, "Couldn't delete the file!\n");
								break;
							default:
								break;
						}
					}
				}
			}
		}
	}

	delete_cached_topics(subscribed_clients);
	close(tcp_sockfd);
	close(udp_sockfd);
	free(buffer);
	return 0;
}