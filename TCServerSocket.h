// TCServerSocket.h
// Created by xale on 3/29/10
//
// Header file for the server version of the TinyControl protocol socket.
//

#ifndef	TCSERVERSOCKET_H
#define	TCSERVERSOCKET_H

#include <pthread.h>
#include <stdint.h>
#include "queue.h"
#include "TCTypes.h"

typedef struct _TCServerSocket
{
	socket_fd sock;
	pthread_t readThread;
	pthread_t writeThread;
	queue* writeQueue;
	inet_socket_address remoteAddress;
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Creates a new TinyControl server socket that will attempt to establish a connection with a TinyControl client socket at the specified remote address
TCServerSocketRef TCServerSocketCreate(inet_socket_address connectAddress);

// Closes, cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef serverSocket);

// Sends the specified data over a TinyControl server socket
void TCServerSocketSendBytes(TCServerSocketRef serverSocket, const char* data);

// Accessors for data about a TinyControl server socket
uint32_t TCServerSocketGetRemoteAddress(TCServerSocketRef serverSocket);
uint16_t TCServerSocketGetRemotePort(TCServerSocketRef serverSocket);

#endif // TCSERVERSOCKET_H
