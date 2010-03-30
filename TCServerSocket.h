// TCServerSocket.h
// Created by xale on 3/29/10
//
// Header file for the server version of the TinyControl protocol socket.
//

#include <pthread.h>
#include <stdint.h>
#include "TCTypes.h"

#ifndef	TCSERVERSOCKET_H
#define	TCSERVERSOCKET_H

typedef struct _TCServerSocket
{
	socket_fd _sock;
	pthread_t _readThread;
	inet_socket_address _remoteAddress;
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Creates a new TinyControl server socket that will attempt to establish a connection with a TinyControl client socket at the specified remote address
TCServerSocketRef TCServerSocketCreate(inet_socket_address connectAddress);

// Closes, cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef socket);

// Sends the specified data over a TinyControl server socket
void TCServerSocketSendBytes(TCServerSocketRef socket, const char* data);

// Accessors for data about a TinyControl server socket
uint32_t TCServerSocketGetRemoteAddress(TCServerSocketRef socket);
uint16_t TCServerSocketGetRemotePort(TCServerSocketRef socket);

#endif // TCSERVERSOCKET_H
