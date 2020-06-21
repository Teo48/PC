#include "my_parser.h"
#include "skel.h"

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