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
#include "queue.h"
#include "TCTypes.h"

typedef struct TCServerSocket
{
	socket_fd sock;
	socket_address* remoteAddress;
	socket_address_length remoteAddressLength;
	
	pthread_mutex_t* mutex;
	pthread_t readThread;
	bool isReading;
	pthread_t writeThread;
	bool isWriting;
	
	queue* writeQueue;
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Creates a new TinyControl server socket that will attempt to establish a connection with a TinyControl client socket at the specified remote address
TCServerSocketRef TCServerSocketCreate(const socket_address* connectAddress, socket_address_length addressLength);

// Closes, cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef serverSocket);

// Sends the specified data over a TinyControl server socket
void TCServerSocketSend(TCServerSocketRef serverSocket, const char* data, size_t dataLength);

// Retrieves the string version of the server socket's connected host's IP address
socket_address* TCServerSocketGetRemoteAddress(TCServerSocketRef serverSocket);

#endif // TCSERVERSOCKET_H
