#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "TCTypes.h"
#include "TCPacket.h"
#include "queue.h"
#include "TCUtilities.h"

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
		fprintf(stderr, "Failed to lookup address and port given\n");
		// gai_strerror();
		return -1;
	}
	int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == -1)
	{
		fprintf(stderr, "Failed to open socket.\n");
		return -1;
	}
	status = sendto(sock, TC_HANDSHAKE_SYN_MSG, strlen(TC_HANDSHAKE_SYN_MSG), 0,
			result->ai_addr, sizeof (struct sockaddr_storage));
	// TODO: anticipate failure
	{
		char* str = TCPrintAddress(result->ai_addr);
		fprintf(stderr, "Sent %d bytes of %zu to %s.\n",
				status, strlen(TC_HANDSHAKE_SYN_MSG), str);
		free(str);
	}
	struct sockaddr_storage server;
	unsigned int fromlen;
	char buf[TC_HANDSHAKE_BUFFER_SIZE+1];
	status = recvfrom(sock, buf, TC_HANDSHAKE_BUFFER_SIZE, 0,
			(struct sockaddr*) &server, &fromlen);
	{
		char* str = TCPrintAddress((struct sockaddr*) &server);
		fprintf(stderr, "Received %d bytes: \"%s\" from %s.\n",
				status, buf, str);
		free(str);
	}
	if (!strncmp(buf, TC_HANDSHAKE_SYNACK_MSG, strlen(TC_HANDSHAKE_SYNACK_MSG)))
	{
		// not a synack
		return -1;
	}
	status = connect(sock, (struct sockaddr*) &server, fromlen);
	// TODO: anticipate failure

	status = send(sock, TC_HANDSHAKE_ACK_MSG, strlen(TC_HANDSHAKE_ACK_MSG), 0);
	{
		fprintf(stderr, "Sent %d bytes of %zu.\n",
				status, strlen(TC_HANDSHAKE_SYNACK_MSG));
	}

	// return a connected sockfd
	return sock;
}

void ntoh_data_packet(uint8_t *buf, data_packet *data)
{
	data->seq_number = ntohl((uint32_t) *buf);
	data->timestamp  = ntohl((uint32_t) *(buf + 4));
	data->rtt		 = ntohl((uint32_t) *(buf + 8));
	memcpy(data->payload, buf + 12, MAX_PAYLOAD_SIZE);
}

int reader(int sock, struct queue* q)
{
	uint8_t data_buffer[sizeof(data_packet)];
	data_packet data;
	int received;
	int flag;
	memset(&data_buffer, 0, sizeof(data_packet));
	received = recv(sock, &data_buffer, sizeof(data_packet), 0);
	if (received <= 0)
	{
		// no data read; error or connection close.
		return -1;
	}
	else if (received < (int) sizeof(data_packet))
	{
		// short packet
		flag = 0;
	}
	ntoh_data_packet(data_buffer, &data);
}

