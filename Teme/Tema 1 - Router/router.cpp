#include "skel.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <list>

#define IP_OFF (sizeof(struct ether_header))
#define ICMP_OFF (IP_OFF + sizeof(struct iphdr))
#define NMAX 700000
#define MAX_LEN_WORD 20

struct route_table_entry {
	uint32_t prefix;
	uint32_t next_hop;
	uint32_t mask;
	int interface;
} __attribute__((packed));

struct route_table_entry *rtable;
int rtable_size;

int read_rtable(struct route_table_entry *rtable) {
	freopen("rtable.txt", "r", stdin);
	char prefix[MAX_LEN_WORD], next_hop[MAX_LEN_WORD], mask[MAX_LEN_WORD], interface[MAX_LEN_WORD];
    int i = 0;
    
    for (; scanf("%s%s%s%s", prefix, next_hop, mask, interface) > 0; ++i) {
        rtable[i].prefix = inet_addr(prefix);
        rtable[i].next_hop = inet_addr(next_hop);
        rtable[i].mask = inet_addr(mask);
        rtable[i].interface = atoi(interface);
    }

   return i;
}

struct route_table_entry *get_best_route(__u32 dest_ip) {
	int i, step;
	for (step = 1 ; step < rtable_size ; step <<= 1);
	for (i = 0 ; step; step >>= 1) {
		if (i + step < rtable_size && rtable[i + step].prefix <= (dest_ip & rtable[i + step].mask))
			i += step;
	}

	if (i != 0) {
		return &rtable[i];
	}
    return NULL;
}

packet get_icmp_pckg(ether_header *eth_hdr, iphdr *ip_hdr, int i, int icmp_type){
			packet icmp_reply_pckg;
			icmp_reply_pckg.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);
			struct ether_header *eth_reply = (struct ether_header *)icmp_reply_pckg.payload;
			struct iphdr *ip_reply= (struct iphdr *)(icmp_reply_pckg.payload + IP_OFF);
			struct icmphdr *icmp_reply = (struct icmphdr *)(icmp_reply_pckg.payload + ICMP_OFF);
			eth_reply->ether_type = htons(ETHERTYPE_IP);
			ip_reply->frag_off = 0;
			ip_reply->ihl = 5;
			ip_reply->version = 4;
			ip_reply->daddr = ip_hdr->saddr;
			ip_reply->saddr = ip_hdr->daddr;
			ip_reply->protocol = IPPROTO_ICMP;
			ip_reply->ttl = 64;
			ip_reply->id = htons(getpid());
			ip_reply->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
			ip_reply->check = 0;

			memcpy(eth_reply->ether_shost, eth_hdr->ether_dhost, sizeof(eth_hdr->ether_dhost));
			memcpy(eth_reply->ether_dhost, eth_hdr->ether_shost, sizeof(eth_hdr->ether_shost));
			ip_reply->check = ip_checksum(ip_reply,sizeof(struct iphdr));
			icmp_reply->type = icmp_type;
			icmp_reply->code = 0;
			icmp_reply->un.echo.id = htons(getpid());
			icmp_reply->un.echo.sequence = htons(i);
			icmp_reply->checksum = 0;
			icmp_reply->checksum = ip_checksum(icmp_reply,sizeof(struct icmphdr));

			return icmp_reply_pckg;
}

int main(int argc, char *argv[])
{
	packet m;
	int rc;
	int i = 0;

	init();
	rtable = new route_table_entry[NMAX];
	DIE(rtable == NULL, "memory");
	rtable_size = read_rtable(rtable);
	std::sort(rtable,rtable + rtable_size, [](const route_table_entry& a, const route_table_entry& b) {
		if (a.prefix == b.prefix) {
			return a.mask < b.mask;
		}

		return a.prefix < b.prefix;
	});
	
	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");
		struct ether_header *ethdr = (struct ether_header *) m.payload;
		switch (ntohs(ethdr->ether_type)) {
			case ETHERTYPE_IP: {
				struct iphdr *ip_hdr = (struct iphdr *) (m.payload + sizeof(struct ether_header));
				__u16 old_check_sum = ip_hdr->check;
				ip_hdr->check = 0;
				if (ip_hdr->protocol == IPPROTO_ICMP && ip_hdr->daddr == inet_addr(get_interface_ip(m.interface))) {	
					struct icmphdr *icmp_hdr = (struct icmphdr *)(m.payload + sizeof(struct ether_header) + sizeof(struct iphdr));
					if (icmp_hdr->type == ICMP_ECHO) {
						std::swap(ip_hdr->saddr, ip_hdr->daddr);
						icmp_hdr->type = ICMP_ECHOREPLY;
						send_packet(m.interface, &m);
					}
					break;
				}

				if (old_check_sum != ip_checksum(ip_hdr, sizeof(struct iphdr))) {
					break;
				}

				if (ip_hdr->ttl <= 1) {
					packet reply = get_icmp_pckg(ethdr, ip_hdr, i, ICMP_TIME_EXCEEDED);
					reply.interface = m.interface;
					send_packet(reply.interface, &reply);
					break;
				}

				struct route_table_entry *rte = get_best_route(ip_hdr->daddr);
				if (rte == NULL) {
					packet reply = get_icmp_pckg(ethdr, ip_hdr, i, ICMP_DEST_UNREACH);
					reply.interface = m.interface;
					send_packet(reply.interface, &reply);
					break;
				}

				--ip_hdr->ttl;
				ip_hdr->check = 0;
				ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));	
			
				get_interface_mac(rte->interface, ethdr->ether_shost);
				++i;
				send_packet(rte->interface, &m);
				break;
			}
			
			case ETHERTYPE_ARP: {
				default:
					break;
			}
		}
	}
}