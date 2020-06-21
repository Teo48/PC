#include "parser.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "skel.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>
/* ethheader */
#include <net/ethernet.h>
/* ether_header */
#include <arpa/inet.h>
/* icmphdr */
#include <netinet/ip_icmp.h>
/* arphdr */
#include <net/if_arp.h>
#include <asm/byteorder.h>
#include <stdint.h> 
struct route_table_entry *rtable;

int my_read_rtable(struct route_table_entry *rtable) {
    FILE *f = fopen("rtable.txt", "r");
    DIE(f == NULL, "Couldn't open rtable.txt!\n");

    char line[100];
    printf("DAAAA");
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

int main() {
    
    int d = my_read_rtable(rtable);
}