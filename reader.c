#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "TCTypes.h"
#include "TCPacket.h"
#include "queue.h"
#include "TCUtilities.h"

static int feedback_timer_expired = 1;

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
	status = sendto(sock, TC_HANDSHAKE_SYN_MSG, strlen(TC_HANDSHAKE_SYN_MSG), 0, result->ai_addr, result->ai_addrlen);
	if (status >= 0)
	{
		char* str = TCPrintAddress(result->ai_addr);
		fprintf(stderr, "Sent %d bytes of %zu to %s.\n", status, strlen(TC_HANDSHAKE_SYN_MSG), str);
		free(str);
	}
	else
	{
		perror("Failed to send SYN");
		return -1;
	}
	struct sockaddr_storage server;
	unsigned int fromlen = sizeof(struct sockaddr_storage);
	char buf[TC_HANDSHAKE_BUFFER_SIZE+1];
	memset(buf, 0, TC_HANDSHAKE_BUFFER_SIZE + 1);
	status = recvfrom(sock, buf, TC_HANDSHAKE_BUFFER_SIZE, 0, (struct sockaddr*) &server, &fromlen);
	{
		char* str = TCPrintAddress((struct sockaddr*) &server);
		fprintf(stderr, "Received %d bytes: \"%s\" from %s.\n", status, buf, str);
		free(str);
	}
	if (strncmp(buf, TC_HANDSHAKE_SYNACK_MSG, strlen(TC_HANDSHAKE_SYNACK_MSG)))
	{
		// not a synack
		fprintf(stderr, "Received \"%s\" but expected \"%s\".\n", buf, TC_HANDSHAKE_SYNACK_MSG);
		return -1;
	}
	status = connect(sock, (struct sockaddr*) &server, fromlen);

	fprintf(stderr, "Status on connect is %d.\n", status);

	status = send(sock, TC_HANDSHAKE_ACK_MSG, strlen(TC_HANDSHAKE_ACK_MSG), 0);
	{
		fprintf(stderr, "Sent %d bytes of %zu.\n", status, strlen(TC_HANDSHAKE_SYNACK_MSG));
	}

	// return a connected sockfd
	return sock;
}

void ntoh_data_packet(uint8_t *buf, data_packet *data)
{
	data->seq_number = ntohl(*(uint32_t*) buf);
	data->timestamp  = ntohl(*(uint32_t*) (buf + 4));
	data->rtt		 = ntohl(*(uint32_t*) (buf + 8));
	memcpy(data->payload, buf + 12, MAX_PAYLOAD_SIZE);
}

void hton_feedback_packet(feedback_packet *feedback, uint32_t *buf)
{
	buf[0] = htonl(feedback->timestamp);
	buf[1] = htonl(feedback->elapsed_time);
	buf[2] = htonl(feedback->receive_rate);
	buf[3] = htonl(feedback->loss_event_rate);
}

void *run_timer(void* argv)
{
	long msec = *(long*) argv;
	struct timespec ts = { .tv_sec = msec/1000, .tv_nsec = 1000*(msec % 1000)};
	nanosleep(&ts, NULL);
	feedback_timer_expired = 1;
	return NULL;
}

void feedback_timer(long *msec, pthread_t *tid)
{
	feedback_timer_expired = 0;
	pthread_create(tid, NULL, run_timer, msec);
}

void send_feedback_packet(int sock, void *buf)
{
	if (feedback_timer_expired == 0)
	{
		return;
	}
	send(sock, buf, FEEDBACK_PACKET_SIZE, 0);
	fprintf(stderr, "Sent feedback packet.\n");
}

int reader(int sock, struct queue* q)
{
	uint8_t data_buffer[sizeof(data_packet)];
	uint32_t feedback_buffer[4];
	data_packet data;
	feedback_packet feedback;
	int received;
	int flag = 1;
	int retval = 0;
	uint32_t time = 0;
	uint32_t last_time = 0;
	uint32_t recv_rate = 0;
	uint32_t loss_rate = 0;
	long msecs = 1000;
	pthread_t feedback_timer_tid;
	while (flag)
	{
		memset(&feedback_buffer, 0, FEEDBACK_PACKET_SIZE);
		memset(&data_buffer, 0, DATA_PACKET_SIZE);
		received = recv(sock, &data_buffer, DATA_PACKET_SIZE, 0);
		fprintf(stderr, "Received packet.\n");
		if (received <= 0)
		{
			fprintf(stderr, "No data read; error or connection close.\n");
			retval = -1;
			break;
		}
		else if (received < DATA_PACKET_SIZE)
		{
			fprintf(stderr, "Short packet.\n");
			if (received < DATA_PACKET_HEADER_LENGTH)
			{
				fprintf(stderr, "Incomplete header.\n");
				retval = -1;
				break;
			}
			else if (received == DATA_PACKET_HEADER_LENGTH)
			{
				fprintf(stderr, "Empty packet.\n");
				break;
			}
			flag = 0;
		}
		ntoh_data_packet(data_buffer, &data);
		msecs = data.rtt;
		char* buf = TCPrintDataPacket(&data);
		fprintf(stderr, "%s\n", buf);
		free(buf);

		// FIXME:get time

		feedback.timestamp = data.timestamp;
		feedback.elapsed_time = last_time - time;
		// FIXME:calculate receive rate
		feedback.receive_rate = recv_rate;
		// FIXME:calculate loss rate
		feedback.loss_event_rate = loss_rate;
		hton_feedback_packet(&feedback, feedback_buffer);

		send_feedback_packet(sock, feedback_buffer);
		feedback_timer(&msecs, &feedback_timer_tid);

		push_back(q, received - DATA_PACKET_HEADER_LENGTH, data.payload);
		fprintf(stderr, "Pushed data to queue.\n");
		fprintf(stderr, "Reader's queue contains %d elements.\n", q->count);
	}
	
	return retval;
}

