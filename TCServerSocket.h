// TCServerSocket.h
// Created by xale on 3/29/10
//
// Header file for the server version of the TinyControl protocol socket.
//

#ifndef	TCSERVERSOCKET_H
#define	TCSERVERSOCKET_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include "queue.h"
#include "TCTypes.h"

typedef struct TCServerSocket
{
	socket_fd sock;
	socket_address* remoteAddress;
	socket_address_length remoteAddressLength;
	
	pthread_mutex_t mutex;
	pthread_t readThread;
	bool isReading;
	
	uint32_t sendRate;			// bytes per second
	uint32_t RTT;				// milliseconds
	uint32_t sequenceNumber;
	uint32_t feedbackTimeout;	// milliseconds
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Creates a new TinyControl server socket that will attempt to establish a connection with a TinyControl client socket at the specified remote address
TCServerSocketRef TCServerSocketCreate(const socket_address* connectAddress, socket_address_length addressLength);

// Closes, cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef serverSocket);

// Send the contents of the specified file over a TinyControl server socket
void TCServerSocketSend(TCServerSocketRef serverSocket, file_fd file);

// Retrieves the string version of the server socket's connected host's IP address
socket_address* TCServerSocketGetRemoteAddress(TCServerSocketRef serverSocket);

#endif // TCSERVERSOCKET_H
