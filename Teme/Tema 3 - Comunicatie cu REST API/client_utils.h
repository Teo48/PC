#ifndef __CLIENT_UTILS_H__
#define __CLIENT_UTILS_H__

#define SERVER "3.8.116.10"
#define URL_MAX_LEN 1000
#define MAX_LEN 100

void _register(int sockfd, const char *username, const char *password,
               char **message, char **response);

void login(int sockfd, const char *username, const char *password,
           char **message, char **response, char **cookie);

void enter_library(int sockfd, char **message, char **response,
                   char **cookie, char **auth_token);

void get_books(int sockfd, char **message, char **response,
               char **cookie, char **auth_token, char **command);

void get_book(int sockfd, char **message, char **response,
              char **cookie, char **auth_token, char **command);

void add_book(int sockfd, char **message, char **response,
              char **cookie, char **auth_token);

void delete_book(int sockfd, char **message, char **response, 
                 char **cookie, char **auth_token);

#endif // __CLIENT_UTILS_H__