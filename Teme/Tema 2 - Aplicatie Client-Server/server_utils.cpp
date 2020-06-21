#include "server_utils.h"

// Gestioneaza o comanda de la un client TCP
// subscribed_clients este o mapa ce are drept cheie topicul, iar valorea
// o alta mapa ce are drept cheie id-ul clientului, iar ca valoare fisierele
// corespunzatoare topicurilor la care este abondat
// clients_sockets este o mapa ce asocieaza socket-ul cu id-ul clientului.
int tcp_client_command(str_str_topic_map &subscribed_clients, int_str_map &clients_sockets,
		char *buffer, int sock) {
	// Copiaza din buffer informatiile necesare si afiseaza in consola
	// serverului daca un client s-a abondat / deconecat de la un topic.
	TCP_CLIENT_REQUEST req;
	memset(&req, 0, sizeof(TCP_CLIENT_REQUEST));
	memcpy(&req, buffer, sizeof(TCP_CLIENT_REQUEST));
	std::string tcp_client_topic(req.topic);
	fprintf(stdout, "Client %s %sd %s\n", clients_sockets[sock].c_str(),
		req.request_type, req.topic);

	// I se asocieaza topicului curent o intrare in mapa
	if (subscribed_clients.find(tcp_client_topic) == subscribed_clients.end()) {
		subscribed_clients.insert(std::make_pair(tcp_client_topic, str_topic_map()));
	}

	// Daca s-a primit o comanda de tip subscribe se verifica daca exista creat
	// fisierul binar asociat clientului, daca nu exista se creeaza o intrare
	// in mapa, respectiv fisierul binar asociat.
	if (!strcmp(req.request_type, "subscribe")) {
		int fd = 0;
		if (subscribed_clients[tcp_client_topic].find(clients_sockets[sock]) 
			== subscribed_clients[tcp_client_topic].end()) {
			subscribed_clients[tcp_client_topic].insert(std::make_pair(clients_sockets[sock],
			subscribed_topic(clients_sockets[sock] + "_" + tcp_client_topic, req.SF)));
			fd = open(subscribed_clients[tcp_client_topic][clients_sockets[sock]].
				subscribed_topic_name.c_str(), O_WRONLY | O_CREAT, 0644);
			if (fd < 0) {
				fprintf(stderr, "Couldn't create a file!");
			}
			close(fd);
			return 0;
		} else {
			subscribed_clients[tcp_client_topic][clients_sockets[sock]].SF = req.SF;
			return 0;
		}
		// Daca este o comanda de tip unsubscribe se verifica daca deja a dat unsubscribe
		// sau daca fisierul binar asociat lui a fost deja sters.
	} else if (!strcmp(req.request_type, "unsubscribe")){
		if (subscribed_clients[tcp_client_topic].find(clients_sockets[sock])
				== subscribed_clients[tcp_client_topic].end()) {
			return -1;
		}
		if ((remove(subscribed_clients[tcp_client_topic][clients_sockets[sock]].
			subscribed_topic_name.c_str()) != 0)) {
			return 1;
		}

		subscribed_clients[tcp_client_topic].erase(clients_sockets[sock]);
	}
	return 0;
}

// Parseaza un mesaj venit da la un client UDP si trimite mesajul
// subscriberilor.
// clients_ids este o mapa ce ascociaza id-ului unui client, socket-ul
// acestuia.
// Primii 2 bytes din buffer sunt asociati dimensiunii mesajului,
// urmatorul tipului mesajului, iar restul pentru mesajul efectiv.
void parse_message(str_str_topic_map &subscribed_clients,
					str_int_map &clients_ids, int udp_sockfd) {
	char *buffer = (char *) calloc(BUFLEN, sizeof(char));
	MEM_ALLOC(buffer, buffer == NULL, "Couldn't allocate memory! Exiting...\n");

	UDP_CLIENT_MESSAGE udp_message;
	memset(&udp_message, 0, sizeof(UDP_CLIENT_MESSAGE));
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	
	int n = recvfrom(udp_sockfd, &udp_message, sizeof(UDP_CLIENT_MESSAGE),
			0, (struct sockaddr *) &cli_addr, &clilen);
	MEM_ALLOC(buffer, n < 0, "recv\n");
	std::string topic(udp_message.topic);
	buffer[2] = udp_message.data_type;

	if (subscribed_clients.find(topic) == subscribed_clients.end()) {
		subscribed_clients.insert(std::make_pair(topic, str_topic_map()));
	}

	char *ip_address = strdup(inet_ntoa(cli_addr.sin_addr));
	MEM_ALLOC(ip_address, ip_address == NULL,
		"Couldn't allocate memory! Exiting...\n");
	uint16_t len = strlen(ip_address);
	memcpy(buffer + 3, ip_address, len);
	++len;
	memcpy(buffer + 3 + len, &cli_addr.sin_port, sizeof(uint16_t));
	len += sizeof(uint16_t);
	memcpy(buffer + 3 + len, udp_message.topic, MAX_TOPIC_SIZE);
	len += strlen(buffer + 3 + len) + 1;

	switch (buffer[2]) {
		case INT : {
			memcpy(buffer + 3 + len, udp_message.content, int_size);
			len += int_size;
			break;
		}
		case SHORT_REAL : {
			memcpy(buffer + 3 + len, udp_message.content, short_real_size);
			len += short_real_size;
			break;
		}
		case FLOAT : {
			memcpy(buffer + 3 + len, udp_message.content, float_size);
			len += float_size;
			break;
		}
		case STRING : {
			memcpy(buffer + 3 + len, udp_message.content, MAX_CONTENT_SIZE);
			len += strlen(buffer + 3 + len);
			break;
		}
		default:
			break;
	}

	++len;
	uint16_t nlen = htons(len);
	memcpy(buffer, &nlen, sizeof(uint16_t));
	// Se parcurge mapa de clienti abonati, iar in cazul in care acestia sunt
	// deconectati si au SF activat se scrie in fisierele binare asociate,
	// altfel se trimite mesajul clientilor.
	for (std::map<std::string, subscribed_topic>::iterator
		it = subscribed_clients[topic].begin(); it != subscribed_clients[topic].end() 
		; ++it) {
		
		if (it->second.SF == 49 && clients_ids[it->first] == -1) {
			int fd = open(it->second.subscribed_topic_name.c_str(),
				O_RDWR | O_APPEND);
			write(fd, buffer, BUFLEN);
			close(fd);
			continue;
		}
		send(clients_ids[it->first], buffer, len + sizeof(uint16_t), 0);
	}

	free(ip_address);
	free(buffer);	
}

// Autentifica un nou client
void authenticate_client(str_str_topic_map &subscribed_clients,
	int_str_map &clients_sockets,str_int_map &clients_ids,
	std::string id, int sock) {
	
	int fd = 0;
	char *buffer = (char *) calloc(BUFLEN, sizeof(char));
	MEM_ALLOC(buffer, buffer == NULL,
		"Couldn't allocate memory! Exiting...\n");
	clients_sockets[sock] = id;

	// Daca este vorba de un client deja existent, se parcurge
	// lista de clienti abonati, iar in cazul in care se gasesc
	// clienti ce au SF activat.(S-au deconecat anterior, iar apoi
	// s-au reconectat, acestia primesc mesajele pe care le-au ratat
	// de-a lungul perioadei de deconectare.)
	if (clients_ids.find(id) != clients_ids.end()) {
		clients_ids[id] = sock;

		for (str_str_topic_map::iterator it = subscribed_clients.begin()
			; it != subscribed_clients.end() ; ++it) {
			if (it->second.find(id) != it->second.end()
				&& it->second[id].SF == 49) {
				fd = open(it->second[id].
					subscribed_topic_name.c_str(), O_RDONLY);
				int bytes_read = 0;

				while ((bytes_read = read(fd, buffer, BUFLEN) > 0)) {			
					uint16_t len;
					memcpy(&len, buffer, sizeof(uint16_t));
					len = ntohs(len);
					send(sock, buffer, len + sizeof(uint16_t), 0);
					memset(buffer, 0, BUFLEN);
				}

				close(fd);
				// Se goleste continutul fisierlor binare asociate
				// pentru a economisi spatiu.
				fd = open(it->second[id].
					subscribed_topic_name.c_str(), O_WRONLY | O_TRUNC);
				close(fd);
			}
		}
	} else {
		clients_ids.insert(std::make_pair(id, sock));
	}
	free(buffer);
}

// Se sterg toate topicurile existente pe disc in momentul in care serverul
// trimite comanda exit
void delete_cached_topics(str_str_topic_map &subscribed_clients) {
	for (str_str_topic_map::iterator it = subscribed_clients.begin() ;
		// Daca este o comanda de tip unsubscribe 
		it != subscribed_clients.end() ; ++it) {
		for (std::map<std::string, subscribed_topic>::iterator itt = it->second.begin();
			itt != it->second.end() ; ++itt) {
			if ((remove(itt->second.subscribed_topic_name.c_str()) != 0)) {
				printf("Couldn't erase file\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}