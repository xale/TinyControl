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
#include "TCUtilities.h"

TCListenSocketRef TCListenSocketCreate(const char* listenPort)
{
	// Look up the necessary address information to create a local listen socket
	inet_address_info addressInfo;
	if (TCGetAddressInfo(NULL, listenPort, AI_PASSIVE, &addressInfo) != 0)
	{
		perror("ERROR: address lookup failed in TCListenSocketCreate()");
		return NULL;
	}
	
	// Create the socket
	socket_fd newSocket = socket(addressInfo.ai_family, addressInfo.ai_socktype, addressInfo.ai_protocol);
	if (newSocket < 0)
	{
		perror("ERROR: socket creation failed in TCListenSocketCreate()");
		return NULL;
	}
	
	// Bind the socket to the specified port
	char* addr = TCPrintAddress(addressInfo.ai_addr);
	printf("DEBUG: binding listen socket to %s\n", addr);
	free(addr);
	if (bind(newSocket, addressInfo.ai_addr, addressInfo.ai_addrlen) != 0)
	{
		perror("ERROR: bind failed in TCListenSocketCreate()");
		return NULL;
	}
	
	// Create the listen socket wrapper struct
	TCListenSocketRef listenSocket = malloc(sizeof(TCListenSocket));
	if (listenSocket == NULL)
	{
		perror("ERROR: malloc failed in TCListenSocketCreate()");
		return NULL;
	}
	
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
				socket_address clientAddress;
				socket_address_length addressLength = sizeof(socket_address);
				ssize_t bytesRead = recvfrom(listenSocket->sock, readBuffer, TC_HANDSHAKE_BUFFER_SIZE, 0, &clientAddress, &addressLength);
				if (bytesRead < 0)
				{
					perror("ERROR: recvfrom() failed in TCListenSocketAccept()");
					break;
				}
				
				// NULL-terminate the string
				readBuffer[bytesRead] = 0;
				
				char* addrBuf = TCPrintAddress(&clientAddress);
				printf("       received %zd bytes from %s: %s\n", bytesRead, addrBuf, readBuffer);
				free(addrBuf);
				
				// Check that the message is a connection request
				if (strncmp(readBuffer, TC_HANDSHAKE_SYN_MSG, TC_HANDSHAKE_BUFFER_SIZE) != 0)
				{
					// Should never happen...
					printf("ERROR: listen socket received non-SYN message from incoming client\n");
					break;
				}
				
				// If the message is a connection request, attempt to complete the connection
				serverSocket = TCServerSocketCreate(&clientAddress, addressLength);
			}
			else
			{
				// Should never happen...
				printf("ERROR: listen socket file descriptor not ready for reading after successful select() operation\n");
			}
			break;
	}
	
	return serverSocket;
}
