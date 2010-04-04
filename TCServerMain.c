// TCServerMain.c
// Created by xale on 3/30/10
//
// Main driver program for the TinyControl protocol server implementation
//

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "TCListenSocket.h"
#include "TCServerSocket.h"
#include "TCUtilities.h"

void usage();

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		usage();
		return EXIT_FAILURE;
	}
	
	// Check that the file exists
	file_fd testFD = open(argv[2], O_RDONLY);
	if (testFD < 0)
	{
		perror("ERROR: could not open file %s");
		return EXIT_FAILURE;
	}
	close(testFD);
	
	// Create a listen socket
	TCListenSocketRef listenSocket = TCListenSocketCreate(argv[1]);
	if (listenSocket == NULL)
	{
		fprintf(stderr, "ERROR: could not initialize socket on port %s; aborting\n", argv[1]);
		return EXIT_FAILURE;
	}
	
	// Listen for incoming connections
	printf("Accepting incoming connections on port %s\n", argv[1]);
	time_delta listenTimeout = {5, 0};
	TCServerSocketRef serverSocket;
	while (true)
	{	
		serverSocket = TCListenSocketAccept(listenSocket, listenTimeout);
		
		if (serverSocket != NULL)
		{
			char* buf = TCPrintAddress(TCServerSocketGetRemoteAddress(serverSocket));
			printf("  Connection accepted from %s\n", buf);
			free(buf);
			
			// Open the file for reading
			file_fd file = open(argv[2], O_RDONLY);
			
			// Send the file to the client
			TCServerSocketSend(serverSocket, file);
			
			// Close the file
			close(file);
			
			// Free the server socket
			TCServerSocketDestroy(serverSocket);
		}
	}
	
	return EXIT_SUCCESS;
}

void usage()
{
	printf("usage: server <listen port> <filename>\n");
}
