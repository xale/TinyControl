// TCServerSocket.c
// Created by xale on 3/30/10
//
// Implementation file for TCServerSocket
//

#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "TCServerSocket.h"

// Private "methods"
// Attempts to perform the second and third stages of a connection handshake; returns a socket file descriptor connected to the client if successful, or -1 in the event of an error.
socket_fd TCServerSocketConnect(inet_socket_address connectAddress);

// Manages the congestion feedback/flow control of the specified socket; run as a separate thread.
void* TCServerSocketReadThread(void* serverSocket);

// Manages the sending of queued outgoing data on the specified socket; run as a separate thread.
void* TCServerSocketWriteThread(void* serverSocket);

TCServerSocketRef TCServerSocketCreate(inet_socket_address connectAddress)
{
	// Attempt to finalize the connection with the client
	socket_fd connectedSocket = TCServerSocketConnect(connectAddress);
	
	// If the connection fails, bail
	if (connectedSocket < 0)
		return NULL;
	
	// Create the server socket object
	TCServerSocketRef serverSocket = malloc(sizeof(TCServerSocket));
	if (serverSocket == NULL)
		return NULL;
	memset(serverSocket, 0, sizeof(TCServerSocket));
	serverSocket->sock = connectedSocket;
	serverSocket->remoteAddress = connectAddress;
	
	// Create a queue for pending writes to the socket
	serverSocket->writeQueue = init_queue();
	if (serverSocket->writeQueue == NULL)
	{
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
	// Create threads to handle writing data to and reading congestion feedback information from the client
	int readRC, writeRC;
	readRC = pthread_create(&(serverSocket->readThread), NULL, TCServerSocketReadThread, (void*)serverSocket);
	writeRC = pthread_create(&(serverSocket->writeThread), NULL, TCServerSocketWriteThread, (void*)serverSocket);
	if ((readRC != 0) || (writeRC != 0))
	{
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
	// If everything looks good so far, return the new socket, ready for writing
	return serverSocket;
}

socket_fd TCServerSocketConnect(inet_socket_address connectAddress)
{
	// Attempt to create a UDP socket
	socket_fd newSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (newSocket < 0)
		return -1;
	
	// Send a reply to the client host's connection request (automatically binds local socket)
	sendto(newSocket, TC_HANDSHAKE_SYNACK_MSG, sizeof(TC_HANDSHAKE_SYNACK_MSG), 0, (struct sockaddr*)&connectAddress, sizeof(connectAddress));
	
	// Create read a file-descriptor-set for select()ing on the socket
	fd_set readFDs;
	FD_ZERO(&readFDs);
	
	// Add the socket to the set
	FD_SET(newSocket, &readFDs);
	
	// Select on the socket, waiting for the client to ACK the connection
	struct timeval timeout = TC_HANDSHAKE_TIMEOUT;
	switch (select((int)newSocket + 1, &readFDs, NULL, NULL, &timeout))
	{
		case -1:
		case 0:
			// Socket error, or connection timed out; connection failed
			return -1;
		
		default:
			if (FD_ISSET(newSocket, &readFDs) != 0)
			{
				// Read the client's message
				char readBuffer[TC_HANDSHAKE_BUFFER_SIZE + 1];
				size_t bytesRead = recv(newSocket, readBuffer, TC_HANDSHAKE_BUFFER_SIZE, 0);
				readBuffer[bytesRead] = 0;
				
				// Check that the message is an ACK
				if (strncmp(readBuffer, TC_HANDSHAKE_ACK_MSG, TC_HANDSHAKE_BUFFER_SIZE) != 0)
				{
					// Not an ACK; connection failed
					return -1;
				}
			}
			else
			{
				// Should never happen...
				printf("WARNING: client socket file descriptor not ready for reading after successful select() operation");
				return -1;
			}
			break;
	}
	
	// Return the connected socket
	return newSocket;
}

void TCServerSocketDestroy(TCServerSocketRef serverSocket)
{
	// Shut down the read and write threads
	// FIXME: WRITEME
	
	// Free the write queue
	// FIXME: free_queue(serverSocket->writeQueue);
	
	// Free the socket struct itself
	free(serverSocket);
}

void TCServerSocketSendBytes(TCServerSocketRef serverSocket, const char* data)
{
	// FIXME: WRITEME
}

void* TCServerSocketReadThread(void* serverSocket)
{
	// FIXME: WRITEME
	
	pthread_exit(NULL);
}

void* TCServerSocketWriteThread(void* serverSocket)
{
	// FIXME: WRITEME
	
	pthread_exit(NULL);
}

uint32_t TCServerSocketGetRemoteAddress(TCServerSocketRef serverSocket)
{
	return serverSocket->remoteAddress.sin_addr.s_addr;
}

uint16_t TCServerSocketGetRemotePort(TCServerSocketRef serverSocket)
{
	return serverSocket->remoteAddress.sin_port;
}
