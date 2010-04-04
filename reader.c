#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "TCTypes.h"

int lookup(char* address, char* port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	struct addrinfo* result;
	int status = getaddrinfo(address, port, &hints, &result);
	if (status != 0)
	{
		// failed to lookup address and port given
		// gai_strerror();
		return -1;
	}
	int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == -1)
	{
		// failed to open socket
		return -1;
	}
	sendto(sock, TC_HANDSHAKE_SYN_MSG, strlen(TC_HANDSHAKE_SYN_MSG), 0,
			result->ai_addr, sizeof (struct sockaddr_storage));
	// TODO: anticipate failure
	struct sockaddr_storage server;
	unsigned int fromlen;
	char buf[TC_HANDSHAKE_BUFFER_SIZE+1];
	recvfrom(sock, &buf, TC_HANDSHAKE_BUFFER_SIZE, 0,
			(struct sockaddr*) &server, &fromlen);
	// TODO: anticipate failure

	connect(sock, (struct sockaddr*) &server, fromlen);
	// TODO: anticipate failure

	send(sock, TC_HANDSHAKE_ACK_MSG, strlen(TC_HANDSHAKE_ACK_MSG), 0);
}
