#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <signal.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include "client_utils.h"

int main(int argc, char *argv[]) {
    char *message = NULL;
    char *response = NULL;
    int sockfd;
    signal(SIGPIPE, SIG_IGN);
    
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }

    char *cookie = NULL;
    char *auth_token = NULL;
    char buff_command[MAX_LEN];
    char username[MAX_LEN];
    char password[MAX_LEN];
    memset(buff_command, 0 ,sizeof(buff_command));
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    while (1) {
        memset(buff_command, 0 ,sizeof(buff_command));
        scanf(" %[^\n]s", buff_command);
        char *command = strdup(buff_command);

        if (strcmp(command, "register") && strcmp(command, "login") && strcmp(command, "enter_library") &&
            strcmp(command, "get_books") && strcmp(command, "get_book") && strcmp(command, "add_book") &&
            strcmp(command, "logout") && strcmp(command, "exit") && strcmp(command, "delete_book")) {
                fprintf(stderr, "Comanda invalida!\n");
                free(command);
                continue;
            }

        if (strcmp(command, "register") == 0) {
            if (cookie != NULL) {
                fprintf(stderr, "Eroare! Nu poti inregistra un alt cont daca esti logat!\n");
                free(command);
                continue;
            }
            fprintf(stdout, "username="), scanf("%s", username);
            fprintf(stdout, "password="), scanf("%s", password);

            _register(sockfd, username, password, &message, &response);
            memset(username, 0, sizeof(username));
            memset(password, 0, sizeof(password));
        }

        if (strncmp(command, "login", 5) == 0) {
            fprintf(stdout, "username="), scanf("%s", username);
            fprintf(stdout, "password="), scanf("%s", password);

            if (cookie != NULL) {
                fprintf(stderr, "Utilizatorul %s este deja logat!\n", username);
                free(command);
                continue;
            }

            login(sockfd, username, password, &message, &response, &cookie);
            memset(username, 0, sizeof(username));
            memset(password, 0, sizeof(password));
        }

        if (strcmp(command, "enter_library") == 0) {
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti autentificat!\n");
                free(command);
                continue;
            }

            enter_library(sockfd, &message, &response, &cookie, &auth_token);
        }
        
        if (strcmp(command, "get_books") == 0) {
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti logat!\n");
                free(command);
                continue;
            }

            if (auth_token == NULL) {
                fprintf(stderr, "Eroare! Nu aveti acces la biblioteca!\n");
                free(command);
                continue;
            }

            get_books(sockfd, &message, &response, &cookie, &auth_token, &command);
        }
        
        if (strcmp(command, "get_book") == 0) {
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti logat!\n");
                free(command);
                continue;
            }

            if (auth_token == NULL) {
                fprintf(stderr, "Eroare! Nu aveti acces la biblioteca!\n");
                free(command);
                continue;
            }

            get_book(sockfd, &message, &response, &cookie, &auth_token, &command);
        }
        

        if (strcmp(command, "add_book") == 0) {
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti logat!\n");
                free(command);
                continue;
            }

            if (auth_token == NULL) {
                fprintf(stderr, "Eroare! Nu aveti acces la biblioteca!\n");
                free(command);
                continue;
            }

            add_book(sockfd, &message, &response, &cookie, &auth_token);
        }

        if (strcmp(command, "delete_book") == 0) {
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti logat!\n");
                free(command);
                continue;
            }
            
            if (auth_token == NULL) {
                fprintf(stderr, "Eroare! Nu aveti acces la biblioteca!\n");
                free(command);
                continue;
            }

            delete_book(sockfd, &message, &response, &cookie, &auth_token);
        }

        if (strcmp(command, "logout") == 0) {
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti logat!\n");
                free(command);
                continue;
            }
            
            char *cookies[1] = {cookie};
            sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
            message = compute_get_request(SERVER, "/api/v1/tema/auth/logout", NULL, cookies, auth_token, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            fprintf(stdout, "V-ati delogat cu succes!\n");
            close_connection(sockfd);
            if (cookie != NULL) {
                free(cookie);
                cookie = NULL;
            }

            if (auth_token != NULL) {
                free(auth_token);
                auth_token = NULL;
            }
        }

        if (strcmp(command, "exit") == 0) {
            free(command);
            break;
        }

        if (response != NULL) {
            free(response);
            response = NULL;
        }

        if (message != NULL) {
            free(message);
            message = NULL;
        }

        free(command);
    }
    
    free(auth_token);
    free(cookie);
    close_connection(sockfd);
    return 0;
}
