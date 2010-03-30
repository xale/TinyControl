// TCListenSocket.h
// Created by xale on 3/29/10
//
// Header file for a UDP listener socket that creates and establishes connections over TinyControl server sockets.
//

#include <stdint.h>
#include <sys/time.h>
#include "TCServerSocket.h"
#include "TCTypes.h"

#ifndef TCLISTENSOCKET_H
#define TCLISTENSOCKET_H

typedef struct _TCListenSocket
{
	socket_fd _sock;
} TCListenSocket;

typedef TCListenSocket* TCListenSocketRef;

// Creates a new TinyControl listen socket, listening on the specified port
TCListenSocketRef TCListenSocketCreate(uint16_t listenPort);

// Closes, cleans up and frees a TinyControl listen socket
void TCListenSocketDestory(TCListenSocketRef listenSocket);

// Blocks on accepting an incoming connection on the specified TinyControl listen socket, returning a new TinyControl server socket when a connection is established, or NULL after the specified timeout
TCServerSocketRef TCListenSocketAccept(TCListenSocketRef listenSocket, const struct timeval timeout);

#endif // TCLISTENSOCKET_H
