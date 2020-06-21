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

void _register(int sockfd, const char *username, const char *password, char **message, char **response) {
    char *correct_username = strdup(username);
    char *correct_password = strdup(password);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", correct_username);
    json_object_set_string(root_object, "password", correct_password);
    serialized_string = json_serialize_to_string_pretty(root_value);

    char *content_type = "application/json";
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }
    *message = compute_post_request(SERVER, "/api/v1/tema/auth/register", content_type, serialized_string, NULL, NULL, 0);
    send_to_server(sockfd, *message);
    json_value_free(root_value);
    json_free_serialized_string(serialized_string);
    *response = receive_from_server(sockfd);

    char *error = basic_extract_json_response(*response);
    if (error == NULL) {
        fprintf(stdout, "Utilizatorul %s a fost inregistrat cu succes!\n", correct_username);
    } else {
        fprintf(stdout, "Utilizatorul %s exista deja!\n", correct_username);
    }

    free(correct_password);
    free(correct_username);
    close_connection(sockfd);
}

void login(int sockfd, const char *username, const char *password, char **message, char **response, char **cookie) {
    char *correct_username = strdup(username);
    char *correct_password = strdup(password);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", correct_username);
    json_object_set_string(root_object, "password", correct_password);
    serialized_string = json_serialize_to_string_pretty(root_value);
    
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }
    
    char *content_type = "application/json";
    *message = compute_post_request(SERVER, "/api/v1/tema/auth/login", content_type, serialized_string, NULL, NULL, 0);
    send_to_server(sockfd, *message);
    json_value_free(root_value);
    json_free_serialized_string(serialized_string);
    *response = receive_from_server(sockfd);
    char *error = basic_extract_json_response(*response);
    if (error == NULL) {
        fprintf(stdout, "Utilizatorul %s s-a logat cu succes!\n", correct_username);
        char *start = strstr(*response, "Set-Cookie:");
        char *end = strchr(start, ';');
        *cookie = (char *) calloc((end - start + 13), sizeof(char));
        memcpy(*cookie, start + 12, end - (start + 12));
        fprintf(stdout, "Cookie: %s\n", *cookie);
    } else {
        fprintf(stderr, "Credentiale gresite!\n");
    }

    free(correct_password);
    free(correct_username);
    close_connection(sockfd);
}

void enter_library(int sockfd, char **message, char **response,
                   char **cookie, char **auth_token) {
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }

    char *cookies[1] = {*cookie};
    *message = compute_get_request(SERVER, "/api/v1/tema/library/access", NULL, cookies, NULL, 1);
    send_to_server(sockfd, *message);
    *response = receive_from_server(sockfd);
    
    if (*auth_token != NULL) {
        free(*auth_token);
        *auth_token = NULL;
    }

    char *find_token = basic_extract_json_response(*response);
    char *end = strchr(find_token + 10, '}');
    *auth_token = (char *)calloc(end - (find_token + 10) + 1, sizeof(char));
    memcpy(*auth_token, find_token + 10, end - 1 - (find_token + 10));
    fprintf(stdout, "JWT Token: %s\n", *auth_token);
    close_connection(sockfd);
}

void get_books(int sockfd, char **message, char **response,
               char **cookie, char **auth_token, char **command) {
    
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }

    char *cookies[1] = {*cookie};
    *message = compute_get_request(SERVER, "/api/v1/tema/library/books", NULL, cookies, *auth_token, 1);
    send_to_server(sockfd, *message);
    *response = receive_from_server(sockfd);
    
    char *find_books = basic_extract_json_response(*response);

    if (find_books == NULL) {
        fprintf(stderr, "Biblioteca este goala!\n");
        close_connection(sockfd);
        return;
    }

    JSON_Value *root_value = json_parse_string(find_books - 1);
    char *serialized_book_list = json_serialize_to_string_pretty(root_value);
    fprintf(stdout, "%s\n", serialized_book_list);
    json_value_free(root_value);
    json_free_serialized_string(serialized_book_list);
    
    close_connection(sockfd);
}

void get_book(int sockfd, char **message, char **response, char **cookie,
              char **auth_token, char **command) {
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }
    char id_buffer[MAX_LEN];
    memset(id_buffer, 0, sizeof(id_buffer));

    fprintf(stdout, "id=");
    scanf(" %[^\n]s", id_buffer);

    int id;
    id = atoi(id_buffer);

    if (id == 0 && id_buffer[0] != '0') {
        fprintf(stderr, "Eroare! ID-ul introdus nu este un numar!\n");
        return;
    }

    char *cookies[1] = {*cookie};
    char url[URL_MAX_LEN];
    sprintf(url, "/api/v1/tema/library/books/%d", id);
    *message = compute_get_request(SERVER, url, NULL, cookies, *auth_token, 1);
    send_to_server(sockfd, *message);
    *response = receive_from_server(sockfd);
    char *find_book = basic_extract_json_response(*response);
    JSON_Value *root_value = json_parse_string(find_book - 1);
    char *serialized_book = json_serialize_to_string_pretty(root_value);
    
    if (strstr(serialized_book, "error") != NULL) {
        fprintf(stderr, "Eroare! Cartea cu id-ul %d nu exista!\n", id);
    } else {
        fprintf(stdout, "%s\n", serialized_book);
    }

    json_value_free(root_value);
    json_free_serialized_string(serialized_book);
    close_connection(sockfd);
}

void add_book(int sockfd, char **message, char **response, char **cookie,
              char **auth_token) {
    char title[MAX_LEN];
    char author[MAX_LEN];
    char genre[MAX_LEN];
    char publisher[MAX_LEN];
    char page_count_buff[MAX_LEN];
    int page_count;
    printf("Introdu datele:\n");
    fprintf(stdout, "title="), scanf(" %[^\n]s", title);
    fprintf(stdout, "author="), scanf(" %[^\n]s", author);
    fprintf(stdout, "genre="), scanf(" %[^\n]s", genre);
    fprintf(stdout, "publisher="), scanf(" %[^\n]s", publisher);
    fprintf(stdout, "page_count="), scanf(" %[^\n]s", page_count_buff);

    page_count = atoi(page_count_buff);

    if (page_count == 0 && page_count_buff[0] != '0') {
        fprintf(stderr, "Eroare! Page count nu este un numar intreg!\n");
        return;
    }

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_number(root_object, "page_count", page_count);
    json_object_set_string(root_object, "publisher", publisher);

    serialized_string = json_serialize_to_string_pretty(root_value);
    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }

    char *content_type = "application/json";
    char *cookies[1] = {*cookie};
    *message = compute_post_request(SERVER, "/api/v1/tema/library/books", content_type, serialized_string, cookies, *auth_token, 1);
    send_to_server(sockfd, *message);
    json_value_free(root_value);
    json_free_serialized_string(serialized_string);
    *response = receive_from_server(sockfd);

    char *error = strstr(*response, "429");
    if (error != NULL) {
        fprintf(stderr, "Nu poti adauga mai mult de 3 carti intr-un interval de 15 minute!\n");
    } else {
        fprintf(stdout, "Cartea a fost adaugata cu succes!\n");
    }
    close_connection(sockfd);
}

void delete_book(int sockfd, char **message, char **response, char **cookie,
              char **auth_token) {
    printf("id=");

    char id_buffer[MAX_LEN];
    memset(id_buffer, 0, sizeof(id_buffer));

    scanf("%s", id_buffer);

    int id;
    id = atoi(id_buffer);

    if (id == 0 && id_buffer[0] != '0') {
        fprintf(stderr, "Eroare! ID-ul introdus nu este un numar!\n");
        return;
    }

    sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket error!");
        exit(-1);
    }
    
    char *cookies[1] = {*cookie};
    char url[URL_MAX_LEN];
    sprintf(url, "/api/v1/tema/library/books/%d", id);
    *message = compute_delete_request(SERVER, url, NULL, cookies, *auth_token, 1);
    send_to_server(sockfd, *message);
    *response = receive_from_server(sockfd);

    if (strstr(*response, "404")) {
        fprintf(stderr, "Eroare! Cartea cu id-ul %d nu exista!\n", id);
    } else if (strstr(*response, "200")) {
        fprintf(stderr, "Cartea cu id-ul %d a fost stearsa!\n", id);
    }

    close_connection(sockfd);
}