// TCServerMain.c
// Created by xale on 3/30/10
//
// Main driver program for the TinyControl protocol server implementation
//

#include <stdlib.h>
#include <stdio.h>
#include "TCListenSocket.h"
#include "TCServerSocket.h"

void usage();

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		usage();
		return EXIT_FAILURE;
	}
	
	// Create a listen socket
	printf("DEBUG: creating listen socket on port %s\n", argv[1]);
	TCListenSocketRef listenSocket = TCListenSocketCreate(argv[1]);
	if (listenSocket == NULL)
	{
		printf("ERROR: listenSocket initialization failed; aborting\n");
		return EXIT_FAILURE;
	}
	
	// Listen for incoming connections
	struct timeval listenTimeout = {5, 0};
	TCServerSocketRef serverSocket;
	while (true)
	{
		printf("DEBUG: accepting incoming connections...\n");
		
		serverSocket = TCListenSocketAccept(listenSocket, listenTimeout);
		
		if (serverSocket != NULL)
		{
			printf("       connection accepted\n");
		}
		
		TCServerSocketDestroy(serverSocket);
	}
	
	return EXIT_SUCCESS;
}

void usage()
{
	printf("usage: server <listen port>\n");
}
