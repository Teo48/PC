#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>

// MACRO pentru eliberare de memorie si afisare mesaj de eroare
#define MEM_ALLOC(container, assertion, call_description)					\
    if (assertion) {														\
        fprintf(stderr, "ERROR: " call_description);						\
		free(container);													\
        exit(EXIT_FAILURE);													\
    }

// MACRO pentru eliberare de memorie si afisare mesaj de eroare
#define ASSERT(container1, container2, assertion, call_description)			\
    if (assertion) {														\
        fprintf(stderr, "ERROR: " call_description);						\
		free(container1);													\
		free(container2);													\
        exit(EXIT_FAILURE);													\
    }

#define INT        0
#define SHORT_REAL 1
#define FLOAT      2
#define STRING     3

#define MAX_CONTENT_SIZE    1500
#define BUFLEN      		1600
#define MAX_CLIENTS			128
#define MAX_TOPIC_SIZE 		50
#define MAX_CLIENT_ID_SIZE  10
#define MAX_REQUEST_SIZE    12

// Structura folosita in cazul unei cereri de la un client TCP
typedef struct __attribute__((packed)) TCP_CLIENT_REQUEST {
	char request_type[MAX_REQUEST_SIZE];
	char topic[MAX_TOPIC_SIZE + 1];
	char SF;
} TCP_CLIENT_REQUEST;

// Structura folosita pentru mesajele de la un client UDP
typedef struct __attribute__((packed)) UDP_CLIENT_MESSAGE {
	char topic[MAX_TOPIC_SIZE];
	char data_type;
	char content[MAX_CONTENT_SIZE];
} UDP_CLIENT_MESSAGE;

#endif // __UTILS_H__
