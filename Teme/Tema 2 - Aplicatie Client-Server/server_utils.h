#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

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

// Structura pentru a pastra numele unui topic si SF flag.
// By default SF este -1, iar topicul nu are niciun nume/
typedef struct subscribed_topic {
	std::string subscribed_topic_name;
	char SF;

	subscribed_topic() {
		this->subscribed_topic_name = "";
		this->SF = -1;
	}
	subscribed_topic(const std::string& _subscribed_topic_name, const char& _SF)
		: subscribed_topic_name(_subscribed_topic_name), SF(_SF) {}
} subscribed_topic;

// Structura pentru a pastra ip-ul si port-ul unui client.
// By default port-ul este -1, iar ip-ul este gol.
typedef struct ip_port {
	uint16_t port;
	std::string ip;

	ip_port() {
		this->port = -1;
		this->ip = "";
	}

	ip_port(const uint16_t& _port, const std::string& _ip ) : port(_port), ip(_ip) {}
} ip_port;

// Define-uri pentru a usura scriera
typedef std::map<std::string, std::map<std::string,
	subscribed_topic>> str_str_topic_map;
typedef std::map<std::string, int> str_int_map;
typedef std::map<std::string, subscribed_topic> str_topic_map;
typedef std::map<int, std::string> int_str_map;
typedef std::map<int, ip_port> int_port_map;

constexpr uint32_t int_size = 1 + sizeof(uint32_t);
constexpr uint16_t short_real_size = sizeof(uint16_t);
constexpr uint32_t float_size = 1 + sizeof(uint32_t) + sizeof(uint8_t);

// Gestioneaza o comanda de la un client TCP.
int tcp_client_command(str_str_topic_map &subscribed_clients, int_str_map &clients_sockets,
		char *buffer, int sock);

// Parseaza mesajul de la clientul UDP.
void parse_message(str_str_topic_map &subscribed_clients,
					str_int_map &clients_ids, int udp_sockfd);

// Autentifica clientul TCP.
void authenticate_client(str_str_topic_map &subscribed_clients,
	int_str_map &clients_sockets,str_int_map &clients_ids,
	std::string id, int sock);

// Sterge fisierele binare ramase pe disc in momementul inchiderii
// executarii serverului
void delete_cached_topics(str_str_topic_map &subscribed_clients);

#endif // __SERVER_UTILS_H__