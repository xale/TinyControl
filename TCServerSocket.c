// TCServerSocket.c
// Created by xale on 3/30/10
//
// Implementation file for TCServerSocket
//

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "TCServerSocket.h"
#include "TCPacket.h"
#include "TCUtilities.h"

// Private "methods"
// Attempts to perform the second and third stages of a connection handshake; returns a socket file descriptor connected to the client if successful, or -1 in the event of an error.
socket_fd TCServerSocketConnect(const socket_address* connectAddress, socket_address_length addressLength, time_delta* initialRTT);

// Manages the congestion feedback/flow control of the specified socket; run as a separate thread.
void* TCServerSocketReadThread(void* serverSocket);

// Reads a feedback packet from the server socket, returning the number of bytes read, or -1 on error; called by read thread.
ssize_t TCServerSocketReceiveFeedback(TCServerSocketRef serverSocket, feedback_packet* packet);

// Updates the server socket's send rate when the feedback timer expires; called by read thread.
void TCServerSocketNoFeedbackUpdate(TCServerSocketRef serverSocket);

// Updates the server socket's send rate when a feedback packet is received; called by read thread.
void TCServerSocketFeedbackUpdate(TCServerSocketRef serverSocket, feedback_packet* packet);

// Sends a data_packet over the specified socket, returning the number of bytes written, or -1 on error.
ssize_t TCServerSocketSendPacket(TCServerSocketRef serverSocket, data_packet* packet, size_t packetLength);

// Retrieves the current send rate for the server socket
uint32_t TCServerSocketGetSendRate(TCServerSocketRef serverSocket);

// Retrieves the current estimate for round-trip-time, in milliseconds
uint32_t TCServerSocketGetRTT(TCServerSocketRef serverSocket);

// Sets or returns whether or not the server socket's read thread is active
void TCServerSocketSetIsReading(TCServerSocketRef serverSocket, bool reading);
bool TCServerSocketIsReading(TCServerSocketRef serverSocket);

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
	
	// Initialize the rate-limiting data
	// Initial round-trip-time
	serverSocket->RTT = time_to_milliseconds(&initialRTT);
	
	// Initial send rate
	if (serverSocket->RTT > 0)
	{
		serverSocket->sendRate = ((4 * MAX_PAYLOAD_SIZE) / serverSocket->RTT);
		if (serverSocket->sendRate < MAX_PAYLOAD_SIZE)
			serverSocket->sendRate = MAX_PAYLOAD_SIZE;
	}
	else
	{
		serverSocket->sendRate = UINT32_MAX;
	}
	
	// Sequence number zero 
	serverSocket->sequenceNumber = 0;
	
	// Initial feedback timeout (2 seconds)
	serverSocket->feedbackTimeout = 2000;
	
	// Create a mutex for the server socket object
	if (pthread_mutex_init(&(serverSocket->mutex), NULL) != 0)
	{
		perror("ERROR: mutex_init failed in TCServerSocketCreate()");
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
	// Create threads to handle writing data to and reading congestion feedback information from the client
	int rc;
	TCServerSocketSetIsReading(serverSocket, true);
	rc = pthread_create(&(serverSocket->readThread), NULL, TCServerSocketReadThread, (void*)serverSocket);
	if (rc != 0)
	{
		perror("ERROR: thread initialization failed in TCServerSocketCreate()");
		TCServerSocketDestroy(serverSocket);
		return NULL;
	}
	
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
				fprintf(stderr, "ERROR: client socket file descriptor not ready for reading after successful select() operation in TCServerSocketConnect()\n");
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
	
	// Shut down the read thread
	TCServerSocketSetIsReading(serverSocket, false);
	
	// Wait for the thread to stop
	pthread_join(serverSocket->readThread, NULL);
	
	// Free the mutex
	pthread_mutex_destroy(&(serverSocket->mutex));
	
	// Free the client's address
	free(serverSocket->remoteAddress);
	
	// Free the socket struct itself
	free(serverSocket);
}

void TCServerSocketSend(TCServerSocketRef serverSocket, file_fd file)
{
	fprintf(stderr, "DEBUG: starting write\n");
	bool writing = true;
	while (writing && TCServerSocketIsReading(serverSocket))
	{
		// Update the send rate
		uint32_t bytesToSend = TCServerSocketGetSendRate(serverSocket);
		fprintf(stderr, "DEBUG: current send rate: %u bytes per second\n", bytesToSend);
		
		// Write packets until we reach the send rate limit, or run out of data to send
		data_packet packet;
		for (uint32_t bytesSent = 0; writing && TCServerSocketIsReading(serverSocket) && (bytesSent < bytesToSend);)
		{
			// Read a packet's worth of data from the file
			ssize_t bytesRead = read(file, &(packet.payload), MAX_PAYLOAD_SIZE);
			fprintf(stderr, "DEBUG: %zd bytes read from file\n", bytesRead);
			
			// If the read fails, abort sending immediately
			if (bytesRead < 0)
			{
				perror("ERROR: read() failed in TCServerSocketSend()");
				writing = false;
				break;
			}
			
			// If the end of the file has been reached, stop sending after this packet
			if (bytesRead < MAX_PAYLOAD_SIZE)
			{
				writing = false;
			}
			
			// Fill the other fields of the packet
			// Sequence number
			packet.seq_number = htonl(serverSocket->sequenceNumber);
			serverSocket->sequenceNumber++;
			
			// Timestamp
			time_of_day timeNow;
			gettimeofday(&timeNow, NULL);
			packet.timestamp = htonl(time_to_milliseconds(&timeNow));
			
			// Round-trip-time
			packet.rtt = htonl(TCServerSocketGetRTT(serverSocket));
			
			// Attempt to send the packet
			ssize_t bytesWritten = TCServerSocketSendPacket(serverSocket, &packet, (DATA_PACKET_HEADER_LENGTH + bytesRead));
			fprintf(stderr, "DEBUG: %zd bytes written\n", bytesWritten);
			
			// If the send fails, abort sending immediately
			if (bytesWritten < bytesRead)
			{
				if (bytesWritten < 0)
					perror("ERROR: send() failed in TCServerSocketSendPacket()");
				else
					fprintf(stderr, "ERROR: incomplete send in TCServerSocketSendPacket(): %zd of %zd bytes", bytesWritten, bytesRead);
				writing = false;
				break;
			}
			
			// Add the bytes written to the total number of bytes sent during this write event
			bytesSent += (bytesWritten - DATA_PACKET_HEADER_LENGTH);
		}
		
		// Sleep for one second before the next write event
		if (writing && TCServerSocketIsReading(serverSocket))
		{
			fprintf(stderr, "DEBUG: sleeping\n");
			sleep(1);
		}
	}
}

ssize_t TCServerSocketSendPacket(TCServerSocketRef serverSocket, data_packet* packet, size_t packetLength)
{
	return send(serverSocket->sock, (void*)packet, packetLength, 0);
}

void* TCServerSocketReadThread(void* s)
{
	TCServerSocketRef serverSocket = (TCServerSocketRef)s;
	fd_set readFDs;
	time_delta timeout;
	
	while (TCServerSocketIsReading(serverSocket))
	{
		// Add the socket to a file-descriptor-set for select()ing
		FD_ZERO(&readFDs);
		FD_SET(serverSocket->sock, &readFDs);
		
		// Set the "no feedback" timeout
		gettimeofday(&timeout, NULL);
		timeout.tv_sec += (serverSocket->feedbackTimeout / 1000);
		timeout.tv_usec += (serverSocket->feedbackTimeout % 1000);
		timeout.tv_sec += (timeout.tv_usec / 1000000);
		timeout.tv_usec = (timeout.tv_usec % 1000000);
		
		// Wait for data on the socket
		switch(select((serverSocket->sock + 1), &readFDs, NULL, NULL, &timeout))
		{
			case -1:
				perror("ERROR: select failed in TCServerSocketReadThread()");
				TCServerSocketSetIsReading(serverSocket, false);
				break;
				
			case 0:
				// Timeout: update sending rate
				TCServerSocketNoFeedbackUpdate(serverSocket);
				break;
				
			default:
				if (FD_ISSET(serverSocket->sock, &readFDs) != 0)
				{
					// Data available for reading
					feedback_packet packet;
					ssize_t bytesRead = TCServerSocketReceiveFeedback(serverSocket, &packet);
					if (bytesRead != FEEDBACK_PACKET_SIZE)
					{
						if (bytesRead < 0)
							perror("ERROR: read failed in TCServerSocketReceiveFeedback()");
						else
							fprintf(stderr, "ERROR: partial packet received in TCServerSocketReceiveFeedback()");
						TCServerSocketSetIsReading(serverSocket, false);
						break;
					}
					
					// Update sending rate
					TCServerSocketFeedbackUpdate(serverSocket, &packet);
				}
				else
				{
					// Should never happen...
					fprintf(stderr, "ERROR: socket file descriptor not ready for reading after successful select() operation in TCServerSocketReadThread()\n");
					TCServerSocketSetIsReading(serverSocket, false);
				}
				break;
		}
	}
	
	pthread_exit(NULL);
}

ssize_t TCServerSocketReceiveFeedback(TCServerSocketRef serverSocket, feedback_packet* packet)
{
	uint8_t readBuffer[FEEDBACK_PACKET_SIZE];
	ssize_t bytesRead = recv(serverSocket->sock, readBuffer, FEEDBACK_PACKET_SIZE, 0);
	
	if (bytesRead == FEEDBACK_PACKET_SIZE)
	{
		packet->timestamp = ntohl((uint32_t)(*readBuffer));
		packet->elapsed_time = ntohl((uint32_t)(*(readBuffer + TIMESTAMP_FIELD_WIDTH)));
		packet->receive_rate = ntohl((uint32_t)(*(readBuffer + TIMESTAMP_FIELD_WIDTH + ELAPSED_T_FIELD_WIDTH)));
		packet->loss_event_rate = ntohl((uint32_t)(*(readBuffer + TIMESTAMP_FIELD_WIDTH + ELAPSED_T_FIELD_WIDTH + RECV_RATE_FIELD_WIDTH)));
	}
	
	return bytesRead;
}

void TCServerSocketNoFeedbackUpdate(TCServerSocketRef serverSocket)
{
	// FIXME: WRITEME
}

void TCServerSocketFeedbackUpdate(TCServerSocketRef serverSocket, feedback_packet* packet)
{
	// FIXME: WRITEME
}

uint32_t TCServerSocketGetSendRate(TCServerSocketRef serverSocket)
{
	uint32_t rate;
	pthread_mutex_lock(&(serverSocket->mutex));
	rate = serverSocket->sendRate;
	pthread_mutex_unlock(&(serverSocket->mutex));
	return rate;
}

uint32_t TCServerSocketGetRTT(TCServerSocketRef serverSocket)
{
	uint32_t RTT;
	pthread_mutex_lock(&(serverSocket->mutex));
	RTT = serverSocket->RTT;
	pthread_mutex_unlock(&(serverSocket->mutex));
	return RTT;
}

void TCServerSocketSetIsReading(TCServerSocketRef serverSocket, bool reading)
{
	pthread_mutex_lock(&(serverSocket->mutex));
	serverSocket->isReading = reading;
	pthread_mutex_unlock(&(serverSocket->mutex));
}

bool TCServerSocketIsReading(TCServerSocketRef serverSocket)
{
	bool reading;
	pthread_mutex_lock(&(serverSocket->mutex));
	reading = serverSocket->isReading;
	pthread_mutex_unlock(&(serverSocket->mutex));
	return reading;
}

socket_address* TCServerSocketGetRemoteAddress(TCServerSocketRef serverSocket)
{
	return serverSocket->remoteAddress;
}
