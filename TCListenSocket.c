// TCListenSocket.c
// Created by xale on 3/29/10
//
// Implementation file for a UDP listener socket that creates TinyControl server sockets.
//

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include "TCListenSocket.h"

TCListenSocketRef TCListenSocketCreate(const char* listenPort)
{
	// Look up the necessary address information to create a local listen socket
	inet_address_info addressHints;
	inet_address_info_list addressResults;
	memset(&addressHints, 0, sizeof(addressHints));
	addressHints.ai_family = AF_INET;
	addressHints.ai_socktype = SOCK_DGRAM;
	addressHints.ai_flags = AI_PASSIVE; // autofill local IP address
	if (getaddrinfo(NULL, listenPort, &addressHints, &addressResults) != 0)
		return NULL;
	
	// Keep the first of the address lookup results
	inet_address_info result = *addressResults;
	result.ai_next = NULL;
	
	// Free the remaining results
	freeaddrinfo(addressResults);
	
	// Create a new socket
	socket_fd newSocket = socket(result.ai_family, result.ai_socktype, result.ai_protocol);
	
	// Bind the socket to the specified port
	if (bind(newSocket, result.ai_addr, result.ai_addrlen) != 0)
		return NULL;
	
	// Create the listen socket wrapper struct
	TCListenSocketRef listenSocket = malloc(sizeof(TCListenSocket));
	if (listenSocket == NULL)
		return NULL;
	
	// Wrap the socket and return
	listenSocket->sock = newSocket;
	return listenSocket;
}

void TCListenSocketDestory(TCListenSocketRef listenSocket)
{
	// Free the socket wrapper struct
	free(listenSocket);
}

TCServerSocketRef TCListenSocketAccept(TCListenSocketRef listenSocket, const struct timeval timeout)
{
	// FIXME: WRITEME
	return NULL;
}
