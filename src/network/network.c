#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "network.h"
//#include "main.h"

#ifdef _WIN32

//#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>

#define socklen_t int

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#include "protocol.h"

#define SOCKET int
#define closesocket close
#define INVALID_SOCKET -1

#endif

#include "../main.h"

struct Peer {
	struct sockaddr_in addr;
};

static struct Peer broadcast;

static SOCKET sock_lobby = -1;
static int port_lobby;

unsigned long network_local_ip() {
	struct hostent *host;
	host = gethostbyname("127.0.0.1");
	return (*((struct in_addr **) (host->h_addr_list)))->s_addr;
}

unsigned long network_ip(const char *host) {
	struct hostent *hent;
	hent = gethostbyname(host);
	if(!hent)
		return 0;
	return (*((struct in_addr **) (hent->h_addr_list)))->s_addr;
}

int network_poll_udp() {
	fd_set fds;
	struct timeval tv = {
		.tv_sec = 0,
		.tv_usec = 0,
	};
	FD_ZERO(&fds);
	FD_SET(sock_lobby, &fds);
	return select(sock_lobby + 1, &fds, NULL, NULL, &tv);
}

int network_init(int _port_lobby) {
	int broadcast_enabled = 1;
	int reuse_enabled = 1;
	struct hostent *broadcasthost;
	
	if(sock_lobby != INVALID_SOCKET)
		return 0;
	
	#ifdef _WIN32
	
	WSADATA wsaData;
	WORD version;
	
	version = MAKEWORD(2, 0);
	if (WSAStartup(version, &wsaData) != 0) {
		return -1;
	} else if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) {
		WSACleanup();
		return -1;
	}
	#endif
	
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
	};
	
	addr.sin_port = htons(_port_lobby);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if((sock_lobby = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("failed to open lobby socket\n");
		return -1;
	}
	
	setsockopt(sock_lobby, SOL_SOCKET, SO_BROADCAST, (void *) &broadcast_enabled, sizeof(broadcast_enabled));
	setsockopt(sock_lobby, SOL_SOCKET, SO_REUSEADDR, (void *) &reuse_enabled, sizeof(broadcast_enabled));
	
	if(bind(sock_lobby, (struct sockaddr *) &addr, sizeof(addr)) == INVALID_SOCKET) {
		closesocket(sock_lobby);
		printf("failed to bind lobby socket\n");
		return -1;
	}
	
	port_lobby = _port_lobby;
	
	broadcast.addr.sin_family = AF_INET;
	broadcast.addr.sin_port = htons(port_lobby);
	broadcasthost = gethostbyname("255.255.255.255");
	broadcast.addr.sin_addr = **((struct in_addr **) (broadcasthost->h_addr_list));
	//inet_pton(AF_INET, "255.255.255.255", &broadcast.addr.sin_addr);
	
	return 0;
}

void network_close_udp() {
	closesocket(sock_lobby);
	sock_lobby = -1;
}

int network_broadcast_udp(void *buf, size_t bufsize) {
	//printf("sending \n");
	return sendto(sock_lobby, buf, bufsize, 0, (struct sockaddr *) &broadcast.addr, sizeof(broadcast.addr));
}

int network_send_udp(unsigned long to, void *buf, size_t bufsize) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_lobby);
	addr.sin_addr.s_addr = to;
	return sendto(sock_lobby, buf, bufsize, 0, (struct sockaddr *) &addr, sizeof(addr));
}

unsigned long network_recv_udp(void *buf, size_t bufsize) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	
	recvfrom(sock_lobby, buf, bufsize, 0, (struct sockaddr *) &addr, &addrlen);

	return addr.sin_addr.s_addr;	
}

void network_disconnect_tcp(int sock) {
	closesocket(sock);
}

int network_listen_tcp(int port) {
	int sock;
	int flags;
	
	struct sockaddr_in addr;
	
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket");
		return -1;
	}
	
	flags = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, 4);
	
	memset((void *) &addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = 0;

	if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("bind\n");
		return -1;
	}
	
	if(listen(sock, 2) < 0) {
		printf("listen\n");
		closesocket(sock);
		return -1;
	}
	
/*	#ifndef _WIN32
	if((flags = fcntl(sock, F_GETFL, 0)) < 0)
		flags = 0;
	if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		network_disconnect_tcp(sock);
		return -1;
	}
	#else
	u_long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);
	#endif*/

	return sock;
}

int network_accept_tcp(int listensock) {
	int sock;
	
	int addr_len;
	struct sockaddr_in addr;
	addr_len = sizeof(struct sockaddr_in);

	if((sock = accept(listensock, (void *) &addr, (void *) &addr_len)) == INVALID_SOCKET)
		return -1;

/*	#ifndef _WIN32
	int flags;

	if((flags = fcntl(sock, F_GETFL, 0)) < 0)
		flags = 0;
	if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		network_disconnect_tcp(sock);
		return -1;
	}
	#else
	u_long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);
	#endif
	setsockopt(sock, IPPROTO_TCP, SO_LINGER, NULL, 0);*/

	return sock;
}

int network_connect_tcp(unsigned long to, int port) {
	int sock;
	//struct hostent *host;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
	};
	
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	
	/*host = gethostbyname(host_ip);
	addr.sin_addr = **((struct in_addr **) (host->h_addr_list));*/
	
	addr.sin_addr.s_addr = to;
	addr.sin_port=htons(port);
	
	if(connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		closesocket(sock);
		return -1;
	}
	
	return sock;
}

int network_poll_tcp(int sock) {
	fd_set fds;
	struct timeval tv = {
		.tv_sec = 0,
		.tv_usec = 0,
	};
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	return select(sock + 1, &fds, NULL, NULL, &tv);
}

int network_send_tcp(int sock, char *buf, int buflen) {
	int ret = 0;
	do {
		ret += send(sock, buf + ret, buflen - ret, 0);
	} while(ret != buflen);
	
	return ret;
}

int network_recv_tcp(int sock, char *buf, int buflen) {
	int ret = 0;
	do {
		ret += recv(sock, buf + ret, buflen - ret, 0);
	} while(ret != buflen);
	
	return ret;
}

void network_close_tcp(int sock) {
	closesocket(sock);
}

