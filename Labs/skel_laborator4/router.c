#include "skel.h"
#include "my_parser.h"

int interfaces[ROUTER_NUM_INTERFACES];
struct route_table_entry *rtable;
int rtable_size;

struct arp_entry *arp_table;
int arp_table_len;

/*
 Returns a pointer (eg. &rtable[i]) to the best matching route
 for the given dest_ip. Or NULL if there is no matching route.
*/
struct route_table_entry *get_best_route(__u32 dest_ip) {
	/* TODO 1: Implement the function */
	__u32 max = 0;
	struct route_table_entry *best_route = NULL;
	for (int i = 0; i < rtable_size ; ++i) {
		if (__builtin_popcount(rtable[i].mask) > max && (rtable[i].mask & rtable[i].prefix) == (dest_ip & rtable[i].mask)) {
			max = __builtin_popcount(rtable[i].mask);
			best_route = &rtable[i];
		}
	}
	return best_route;
}

/*
 Returns a pointer (eg. &arp_table[i]) to the best matching ARP entry.
 for the given dest_ip or NULL if there is no matching entry.
*/
struct arp_entry *get_arp_entry(__u32 ip) {
    /* TODO 2: Implement */
	for (int i = 0 ; i < arp_table_len ; ++i) {
		if (arp_table[i].ip == ip) {
			return &arp_table[i];
		}
	}

    return NULL;
}
/*
	Bonus: Parsarea tabelei de routare.
*/
int my_read_rtable(struct route_table_entry *rtable) {
    FILE *f = fopen("rtable.txt", "r");
    DIE(f == NULL, "Couldn't open rtable.txt!\n");

    char line[100];

    int i = 0;
    
    for (; fgets(line, sizeof(line), f); ++i) {
        char prefix[20], next_hop[20], mask[20], interface[20];
        sscanf(line, "%s %s %s %s", prefix, next_hop, mask, interface);
        rtable[i].prefix = inet_addr(prefix);
        rtable[i].next_hop = inet_addr(next_hop);
        rtable[i].mask = inet_addr(mask);
        rtable[i].interface = atoi(interface);
    }

   fclose(f);

   return i;
}

int main(int argc, char *argv[])
{
	msg m;
	int rc;

	init();
	rtable = malloc(sizeof(struct route_table_entry) * 100);
	arp_table = malloc(sizeof(struct  arp_entry) * 100);
	DIE(rtable == NULL, "memory");
	// Asta pentru bonus
	// rtable_size = my_read_rtable(rtable); 
	rtable_size = read_rtable(rtable);
	parse_arp_table();
	/* Students will write code here */
	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");
		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));
		/* TODO 3: Check the checksum */
		__u16 old_check_sum = ip_hdr->check;
		ip_hdr->check = 0;

		if (old_check_sum != ip_checksum(ip_hdr, sizeof(struct iphdr))) {
			fprintf(stderr, "Wrong package!\n");
			continue;
		}
		/* TODO 4: Check TTL >= 1 */

		if (ip_hdr->ttl < 1) {
			fprintf(stderr, "Dead package!\n");
			continue;
		}

		/* TODO 5: Find best matching route (using the function you wrote at TODO 1) */

		struct route_table_entry *route = get_best_route(ip_hdr->daddr);
		
		if (route == NULL) {
			fprintf(stderr, "No route!\n");
			continue;
		}

		/* TODO 6: Update TTL and recalculate the checksum */

		--ip_hdr->ttl;
		ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));
		/* TODO 7: Find matching ARP entry and update Ethernet addresses */
		
		struct arp_entry *entry = get_arp_entry(route->next_hop);

		if (entry == NULL) {
			fprintf(stderr, "No arp entry!\n");
			continue;
		}

		get_interface_mac(route->interface, eth_hdr->ether_shost);
		memcpy(eth_hdr->ether_dhost, entry->mac, sizeof(entry->mac));
		
		/* TODO 8: Forward the pachet to best_route->interface */
		send_packet(route->interface, &m);
	}
}
