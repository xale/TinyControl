// TCListenSocket.c
// Created by xale on 3/29/10
//
// Implementation file for a UDP listener socket that creates TinyControl server sockets.
//

#include <stdlib.h>
#include <stdio.h>
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

TCServerSocketRef TCListenSocketAccept(TCListenSocketRef listenSocket, const struct timeval acceptTimeout)
{
	TCServerSocketRef serverSocket = NULL;

	// Create a read-file-descriptor-set for select()ing on the socket
	fd_set readFDs;
	FD_ZERO(&readFDs);
	
	// Add the listen socket to the set
	FD_SET(listenSocket->sock, &readFDs);
	
	// Select on the socket, waiting for a client to send us a SYN request
	struct timeval timeout = acceptTimeout;
	switch (select(listenSocket->sock + 1, &readFDs, NULL, NULL, &timeout))
	{
		case -1:
		case 0:
			// Socket error, or connection timed out; connection failed
			break;
		
		default:
			if (FD_ISSET(listenSocket->sock, &readFDs) != 0)
			{
				// Attempt to read the incoming message, and determine its origin
				char readBuffer[TC_HANDSHAKE_BUFFER_SIZE + 1];
				generic_socket_address clientAddress;
				socket_address_length addressLength;
				size_t bytesRead = recvfrom(listenSocket->sock, readBuffer, TC_HANDSHAKE_BUFFER_SIZE, 0, &clientAddress, &addressLength);
				if (bytesRead <= 0)
					break;
				
				// Check that the incoming connection is IPv4
				if (clientAddress.sa_family != AF_INET)
				{
					printf("WARNING: listen socket received non-IPv4 connection attempt");
					break;
				}
				
				// NULL-terminate the string
				readBuffer[bytesRead] = 0;
				
				// Check that the message is a connection request
				if (strncmp(readBuffer, TC_HANDSHAKE_SYN_MSG, TC_HANDSHAKE_BUFFER_SIZE) != 0)
				{
					// Should never happen...
					printf("WARNING: listen socket received non-SYN message from incoming client");
					break;
				}
				
				// If the message is a connection request, attempt to complete the connection
				serverSocket = TCServerSocketCreate((inet_socket_address*)&clientAddress);
			}
			else
			{
				// Should never happen...
				printf("WARNING: listen socket file descriptor not ready for reading after successful select() operation");
			}
			break;
	}
	
	return serverSocket;
}
