#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
							char **cookies,  const char *auth_token, int cookies_count);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(const char *host, const char *url, const char* content_type, char *body_data, char **cookies,
	const char *auth_token,  const int cookies_count);

char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, const char *auth_token, int cookies_count);
#endif
