#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "helpers.h"
#include <bits/stdc++.h>

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

int main(int argc, char *argv[])
{   
    if(argc != 4)
    {
        printf("\n Usage: %s <ip> <port> <numar>\n", argv[0]);
        return 1;
    }
    
    struct sockaddr_in to_station;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    to_station.sin_family = AF_INET;
	to_station.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &to_station.sin_addr);
    char buf[BUFLEN];
    memset(buf, 0,BUFLEN);

    std::string aux(argv[3]);
    int nr = atoi(aux.c_str());
    sendto(sockfd, aux.data(), aux.length(), 0, (struct sockaddr *) &to_station, sizeof(to_station));
    printf("S-a trimis numarul: %d !\n", nr);
    close(sockfd);

    return 0;
}
