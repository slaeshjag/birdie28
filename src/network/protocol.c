#include "network.h"
#include "protocol.h"

int protocol_send_packet(int sock, Packet *pack) {
	return network_send_tcp(sock, (void *) pack, pack->size);
}

int protocol_recv_packet(int sock, Packet *pack) {
	int size;
	
	if(network_poll_tcp(sock) == 0)
		return -1;
	
	network_recv_tcp(sock, (void *) pack, 4);
	size = pack->size;
	size -= 4;
	if(pack->size - 4)
		network_recv_tcp(sock, ((void *) pack) + 4, size);
	
	return 0;
}
