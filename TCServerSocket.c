// TCServerSocket.c
// Created by xale on 3/30/10
//
// Implementation file for TCServerSocket
//

#include <string.h> // memset()
#include "TCServerSocket.h"

// Private "methods"
// Attempts to perform the second and third stages of a connection handshake; returns a socket file descriptor connected to the client if successful, or -1 in the event of an error.
socket_fd TCServerSocketConnect(inet_socket_address connectAddress);

TCServerSocketRef TCServerSocketCreate(inet_socket_address connectAddress)
{
	// Attempt to finalize the connection with the client
	socket_fd socket = TCServerSocketConnect(connectAddress);
	
	// If the connection fails, bail
	if (socket < 0)
		return NULL;
	
	
}

socket_fd TCServerSockectConnect(inet_socket_address
{
	// Attempt to create a UDP socket
	socket_fd socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket < 0)
		return -1;
	
	// Send a reply to the client host's connection request (automatically binds local socket)
	sendto(socket, TC_HANDSHAKE_SYNACK_MSG, sizeof(TC_HANDSHAKE_SYNACK_MSG), 0, connectAddress, sizeof(connectAddress));
	
	// Create read a file-descriptor-set for select()ing on the socket
	fd_set readFDs, errorFDs;
	FD_ZERO(&readFDs);
	
	// Add the socket to the set
	FD_SET(socket, &readFDs);
	
	// Select on the socket, waiting for the client to ACK the connection
	switch (select((int)socket + 1, &readFDs, NULL, NULL, &TC_HANDSHAKE_TIMEOUT))
	{
		case SOCKET_ERROR:
		case 0:
			// Socket error, or connection timed out; connection failed
			return -1;
		
		default:
			if (FD_ISSET(socket, &readFDs) != 0)
			{
				// Read the client's message
				char readBuffer[TC_HANDSHAKE_BUFFER_SIZE + 1];
				size_t bytesRead = recv(socket, readBuffer, TC_HANDSHAKE_BUFFER_SIZE, 0);
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
	return socket;
}

void TCServerSocketDestroy(TCServerSocketRef socket)
{
	// FIXME: WRITEME
}

void TCServerSocketSendBytes(TCServerSocketRef socket, const char* data)
{
	// FIXME: WRITEME
}

uint32_t TCServerSocketGetRemoteAddress(TCServerSocketRef socket)
{
	return socket->_remoteAddress.s_addr;
}

uint16_t TCServerSocketGetRemotePort(TCServerSocketRef socket)
{
	return socket->_remoteAddress.sin_port;
}
