#ifndef __SUBSCRIBER_UTILS_H__
#define __SUBSCRIBER_UTILS_H__

// Trimite o comanda la server
int send_command(int sockfd,  char *buffer, char *name);
uint32_t decode_int(char *buffer, uint16_t cursor);
uint16_t decode_short_real(char *buffer, uint16_t cursor);
double decode_float(char *buffer, uint16_t cursor);
// Parseaza mesajul venit de la clientul UDP
void parse_udp_client_message(char *buffer);

#endif // __SUBSCRIBER_UTILS_H__