#ifndef __NETWORK_H_
#define __NETWORK_H_

#include <stdlib.h>

int network_init(int port);

void network_close_udp();
int network_broadcast_udp(void *buf, size_t bufsize);
int network_send_udp(unsigned long to, void *buf, size_t bufsize);
unsigned long network_recv_udp(void *buf, size_t bufsize);
int network_poll_udp();

unsigned long network_local_ip();
unsigned long network_ip(const char *host);

void network_disconnect_tcp(int sock);
int network_listen_tcp(int port);
int network_accept_tcp(int listensock);
int network_connect_tcp(unsigned long to, int port);
int network_poll_tcp(int sock);
int network_send_tcp(int sock, char *buf, int buflen);
int network_recv_tcp(int sock, char *buf, int buflen);
void network_close_tcp(int sock);

#endif
