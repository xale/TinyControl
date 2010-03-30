// TCServer.h
// Created by xale on 3/29/10
//
// Header file for the server version of the TinyControl protocol socket.
//

#include <pthread.h>
#include <stdint.h>
#include "TCTypes.h"

#ifndef	TCSERVER_H
#define	TCSERVER_H

typedef struct _TCServerSocket
{
	socket_fd _sock;
	pthread_t _readThread;
	inet_socket_address _socketAddress;
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Blocks on accepting an incoming connection on the specified socket, returning a new TinyControl server socket when a connection is established. 'socket' must be in listen mode before this call is made.
TCServerSocketRef TCServerSocketAccept(socket_fd socket);

// Closes, cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef socket);

// Sends the specified data over a TinyControl server socket
void TCServerSocketSendBytes(TCServerSocketRef socket, const char* data);

// Accessors for data about a TinyControl server socket
uint32_t TCServerSocketGetAddress(TCServerSocketRef socket);
uint16_t TCServerSocketGetPort(TCServerSocketRef socket);

#endif // TCSERVER_H
