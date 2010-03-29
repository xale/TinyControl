// Packet.h
// Created by xale on 3/29/10
//
// Header file for the server version of the TinyControl protocol socket.
//

typedef int UDPSocket;

typedef struct _TCServerSocket
{
	UDPSocket _sock;
	// FIXME: more
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Creates a new TinyControl server socket
// FIXME: arguments
TCServerSocketRef TCServerSocketCreate();

// Cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef socket);

// Sends the specified data over a TinyControl server socket
void TCServerSocketSendBytes(TCServerSocketRef socket, const char* data);
