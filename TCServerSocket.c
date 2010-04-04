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
#include "TCPacket.h"
#include "TCUtilities.h"

// Private "methods"
// Attempts to perform the second and third stages of a connection handshake; returns a socket file descriptor connected to the client if successful, or -1 in the event of an error.
socket_fd TCServerSocketConnect(const socket_address* connectAddress, socket_address_length addressLength, time_delta* initialRTT);

// Manages the congestion feedback/flow control of the specified socket; run as a separate thread.
void* TCServerSocketReadThread(void* serverSocket);

// Manages the sending of queued outgoing data on the specified socket; run as a separate thread.
void* TCServerSocketWriteThread(void* serverSocket);

// Sends a data_packet over the specified socket; called by the write thread.
int TCServerSocketSendPacket(TCServerSocketRef serverSocket, data_packet* packet, size_t payloadLength);

TCServerSocketRef TCServerSocketCreate(const socket_address* connectAddress, socket_address_length addressLength)
{
	// Attempt to finalize the connection with the client
	time_delta initialRTT;
	socket_fd connectedSocket = TCServerSocketConnect(connectAddress, addressLength, &initialRTT);
	
	// If the connection fails, bail
	if (connectedSocket < 0)
	{
		fprintf(stderr, "       connection failed in TCServerSocketCreate()\n");
		return NULL;
	}
	
	// Create the server socket object
	TCServerSocketRef serverSocket = malloc(sizeof(TCServerSocket));
	if (serverSocket == NULL)
	{
		perror("ERROR: serverSocket malloc failed in TCServerSocketCreate()");
		return NULL;
	}
	memset(serverSocket, 0, sizeof(TCServerSocket));
	serverSocket->sock = connectedSocket;
	
	// Copy the client's address
	serverSocket->remoteAddress = malloc(addressLength);
	memcpy(serverSocket->remoteAddress, connectAddress, addressLength);
	serverSocket->remoteAddressLength = addressLength;
	
	// Copy the initial RTT
	serverSocket->RTT = initialRTT;
	
	// Create a queue for pending writes to the socket
	serverSocket->writeQueue = init_queue();
	if (serverSocket->writeQueue == NULL)
	{
		perror("ERROR: writeQueue malloc failed in TCServerSocketCreate()");
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
	// Create a mutex for the server socket object
	if (pthread_mutex_init(&(serverSocket->mutex), NULL) != 0)
	{
		perror("ERROR: mutex_init failed in TCServerSocketCreate()");
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
	// Create threads to handle writing data to and reading congestion feedback information from the client
	int readRC, writeRC;
	readRC = pthread_create(&(serverSocket->readThread), NULL, TCServerSocketReadThread, (void*)serverSocket);
	writeRC = pthread_create(&(serverSocket->writeThread), NULL, TCServerSocketWriteThread, (void*)serverSocket);
	if ((readRC != 0) || (writeRC != 0))
	{
		perror("ERROR: thread initialization failed in TCServerSocketCreate()");
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
	/* FIXME: testing data transfer ================================================================================ */
	data_packet testPacket;
	memset(&(testPacket.payload), 0, MAX_PAYLOAD_SIZE);
	testPacket.seq_number = htonl(0xDEADBEEF);
	testPacket.timestamp = htonl(100);
	testPacket.rtt = htonl(10);
	
	TCServerSocketSendPacket(serverSocket, &testPacket, MAX_PAYLOAD_SIZE);
	/* FIXME: testing data transfer ================================================================================ */
	
	// If everything looks good so far, return the new socket, ready for writing
	return serverSocket;
}

socket_fd TCServerSocketConnect(const socket_address* connectAddress, socket_address_length addressLength, time_delta* initialRTT)
{
	// Attempt to create a UDP socket
	socket_fd newSocket = socket(connectAddress->sa_family, SOCK_DGRAM, 0);
	if (newSocket < 0)
	{
		perror("ERROR: socket creation failed in TCServerSocketConnect()");
		return -1;
	}
	
	// Make note of the time, for the initial RTT calculation
	time_of_day timeSYNACK;
	if (gettimeofday(&timeSYNACK, NULL) != 0)
	{
		perror("ERROR: cannot get time of SYNACK in TCServerSocketConnect()");
		return -1;
	}
	
	// Send a reply to the client host's connection request (automatically binds local socket)
	ssize_t bytesWritten = sendto(newSocket, TC_HANDSHAKE_SYNACK_MSG, strlen(TC_HANDSHAKE_SYNACK_MSG), 0, connectAddress, addressLength);
	if (bytesWritten < 0)
	{
		perror("ERROR: sendto failed in TCServerSocketConnect()");
		return -1;
	}
	if (bytesWritten < (ssize_t)strlen(TC_HANDSHAKE_SYNACK_MSG))
	{
		fprintf(stderr, "ERROR: unable to send full SYNACK message in TCServerSocketConnect()\n");
		return -1;
	}
	
	// Create a read-file-descriptor-set for select()ing on the socket
	fd_set readFDs;
	FD_ZERO(&readFDs);
	
	// Add the socket to the set
	FD_SET(newSocket, &readFDs);
	
	// Select on the socket, waiting for the client to ACK the connection
	time_delta timeout = TC_HANDSHAKE_TIMEOUT;
	switch (select(newSocket + 1, &readFDs, NULL, NULL, &timeout))
	{
		case -1:
			perror("ERROR: select failed in TCServerSocketConnect()");
			return -1;
			
		case 0:
			fprintf(stderr, "ERROR: timed out waiting for ACK in TCServerSocketConnect()\n");
			return -1;
			
		default:
			if (FD_ISSET(newSocket, &readFDs) != 0)
			{
				// Read the client's message
				char readBuffer[TC_HANDSHAKE_BUFFER_SIZE + 1];
				ssize_t bytesRead = recv(newSocket, readBuffer, TC_HANDSHAKE_BUFFER_SIZE, 0);
				if (bytesRead < 0)
				{
					perror("ERROR: recv failed in TCServerSocketConnect()");
					return -1;
				}
				
				// Note the time
				time_of_day timeACK;
				if (gettimeofday(&timeACK, NULL) != 0)
				{
					perror("ERROR: cannot get time of ACK in TCServerSocketConnect()");
					return -1;
				}
				
				// NULL-terminate the message
				readBuffer[bytesRead] = 0;
				
				// Check that the message is an ACK
				if (strncmp(readBuffer, TC_HANDSHAKE_ACK_MSG, TC_HANDSHAKE_BUFFER_SIZE) != 0)
				{
					// Not an ACK; connection failed
					fprintf(stderr, "ERROR: non-ACK response to handshake in TCServerSocketConnect()\n");
					return -1;
				}
				
				// connect() the socket, so that we don't have to call sendto() in future
				if (connect(newSocket, connectAddress, addressLength) != 0)
				{
					perror("ERROR: could not connect socket in TCServerSocketConnect()");
					return -1;
				}
				
				// Calculate the round-trip-time
				time_subtract(initialRTT, &timeACK, &timeSYNACK);
			}
			else
			{
				// Should never happen...
				fprintf(stderr, "ERROR: client socket file descriptor not ready for reading after successful select() operation\n");
				return -1;
			}
			break;
	}
	
	// Return the connected socket
	return newSocket;
}

void TCServerSocketDestroy(TCServerSocketRef serverSocket)
{
	if (serverSocket == NULL)
		return;
	
	// Acquire mutex
	pthread_mutex_lock(&(serverSocket->mutex));
	
	// Shut down the read and write threads
	serverSocket->isReading = false;
	serverSocket->isWriting = false;
	
	// Wait for the threads to stop
	pthread_join(serverSocket->readThread, NULL);
	pthread_join(serverSocket->writeThread, NULL);
	
	// Release mutex
	pthread_mutex_unlock(&(serverSocket->mutex));
	
	// Free the mutex
	pthread_mutex_destroy(&(serverSocket->mutex));
	
	// Free the write queue
	free_queue(serverSocket->writeQueue);
	
	// Free the client's address
	free(serverSocket->remoteAddress);
	
	// Free the socket struct itself
	free(serverSocket);
}

void TCServerSocketSend(TCServerSocketRef serverSocket, const char* data, size_t dataLength)
{
	pthread_mutex_lock(&(serverSocket->mutex));
	
	// Check that the socket is not already writing data
	if (serverSocket->isWriting)
	{
		pthread_mutex_unlock(&(serverSocket->mutex));
		fprintf(stderr, "WARNING: attempt to write to server socket with queued data");
		return;
	}
	
	// Fill the queue with packet payloads
	payload_t nextPayload;
	size_t payloadSize, dataLeft = dataLength;
	while (dataLeft > 0)
	{
		// Calculate the size of this packet payload
		payloadSize = (MAX_PAYLOAD_SIZE > dataLeft) ? dataLeft : MAX_PAYLOAD_SIZE;
		
		// Load the packet
		memcpy(&nextPayload, data + (dataLength - dataLeft), payloadSize);
		
		// Add the packet to the queue
		push_back(serverSocket->writeQueue, payloadSize, nextPayload);
		
		// Subtract the amount of data added to the queue from the amount remaining
		dataLeft -= payloadSize;
	}
	
	// Start the server writing
	serverSocket->isWriting = true;
	
	pthread_mutex_unlock(&(serverSocket->mutex));
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

int TCServerSocketSendPacket(TCServerSocketRef serverSocket, data_packet* packet, size_t payloadLength)
{
	return send(serverSocket->sock, (void*)packet, (DATA_PACKET_HEADER_LENGTH + payloadLength), 0);
}

socket_address* TCServerSocketGetRemoteAddress(TCServerSocketRef serverSocket)
{
	return serverSocket->remoteAddress;
}
