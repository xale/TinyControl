// Packet.h
// Created by xale on 3/29/10
//
// Header file for the server version of the TinyControl protocol socket.
//

#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>

#ifndef	TCSERVER_H
#define	TCSERVER_H

typedef int udp_socket;
typedef struct sockaddr_in inet_socket_address;

typedef struct _TCServerSocket
{
	udp_socket _sock;
	pthread_t _readThread;
	inet_socket_address _socketAddress;
} TCServerSocket;

typedef TCServerSocket* TCServerSocketRef;

// Creates a new TinyControl server socket
TCServerSocketRef TCServerSocketCreate(const char* address, uint16_t listenPort);

// Cleans up and frees a TinyControl server socket
void TCServerSocketDestroy(TCServerSocketRef socket);

// Sends the specified data over a TinyControl server socket
void TCServerSocketSendBytes(TCServerSocketRef socket, const char* data);

// Accessors for data about a TinyControl server socket
uint32_t TCServerSocketGetAddress(TCServerSocketRef socket);
uint16_t TCServerSocketGetPort(TCServerSocketRef socket);

#endif	// TCSERVER_H
