/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	client mini-server de backup fisiere
*/

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "helpers.h"


void usage(char*file)
{
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);
}

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv)
{
	if (argc!=4)
		usage(argv[0]);
	
	int fd;
	struct sockaddr_in to_station;
	char buf[BUFLEN];

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	/* Deschidere fisier pentru citire */
	DIE((fd=open(argv[3],O_RDONLY))==-1,"open file");

	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(atoi(argv[2])); // acelasi ca in server
	inet_aton(argv[1], &to_station.sin_addr);


	/*
	*  cat_timp  mai_pot_citi
	*		citeste din fisier
	*		trimite pe socket
	*/	
	memset(buf,0,BUFLEN);
	int x;
	while((x = read(fd,buf,BUFLEN))){
		sendto(sockfd, buf, x, 0, (struct sockaddr *) &to_station, sizeof(to_station));
		usleep(2000);
		memset(buf,0,BUFLEN);
	}
	memset(buf,0,BUFLEN);
	sendto(sockfd, "ack", 4, 0, (struct sockaddr *) &to_station, sizeof(to_station));
	memset(buf,0,BUFLEN);

	close(sockfd);
	close(fd);
	return 0;
}