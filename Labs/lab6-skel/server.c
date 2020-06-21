/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	mini-server de backup fisiere
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
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port nume_fisier
*/
int main(int argc,char**argv)
{
	int fd;

	if (argc!=3)
		usage(argv[0]);
	
	struct sockaddr_in my_sockaddr, from_station ;
	char buf[BUFLEN];


	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	
	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */

	from_station.sin_family = AF_INET;
	from_station.sin_port = htons(atoi(argv[1]));
	from_station.sin_addr.s_addr = INADDR_ANY;
	/* Legare proprietati de socket */
	// int enable = 1;
	// if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
 //   		perror("setsocketopt");
 //   		exit(1);
 // }


	bind(sockfd,(struct sockaddr *) &from_station,sizeof(from_station));
	
	/* Deschidere fisier pentru scriere */
	DIE((fd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644))==-1,"open file");
	
	/*
	*  cat_timp  mai_pot_citi
	*		citeste din socket
	*		pune in fisier
	*/
	socklen_t client_len;
	memset(buf,0,BUFLEN);
	while(1){
		int x=recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &from_station, &client_len);
		if(strcmp(buf,"ack")==0)
			break; 
		write(fd,buf,x);
		memset(buf,0,BUFLEN);
	}

	
		close(sockfd);
	
	close(fd);
	return 0;
}