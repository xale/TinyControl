/*
 *  TCUtilities.c
 *  TinyControl
 *
 *  Created by xale on 4/3/10.
 *
 *	Utility functions for TinyControl protocol
 */

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "TCUtilities.h"

int TCGetAddressInfo(const char* hostname, const char* port, int flags, inet_address_info* result)
{
	if (result == NULL)
		return -1;
	
	// Create the address lookup "hints": IPv4 or v6, UDP-only
	inet_address_info addressHints;
	memset(&addressHints, 0, sizeof(addressHints));
	addressHints.ai_family = AF_UNSPEC;
	addressHints.ai_socktype = SOCK_DGRAM;
	addressHints.ai_flags = flags;
	
	// Look up the address information for the host
	inet_address_info_list addressResults;
	if (getaddrinfo(hostname, port, &addressHints, &addressResults) != 0)
		return -1;

	// Keep the first of the address lookup results
	memcpy(result, addressResults, sizeof(inet_address_info));
	result->ai_next = NULL;

	// Free the remaining results
	freeaddrinfo(addressResults);
	
	return 0;
}

const char* TCAddressToString(socket_address* address)
{
	char* buffer;
	switch(address->sa_family)
	{
        case AF_INET:
			buffer = malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(((struct sockaddr_in *)address)->sin_addr), buffer, INET_ADDRSTRLEN);
            break;
			
        case AF_INET6:
			buffer = malloc(INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)address)->sin6_addr), buffer, INET6_ADDRSTRLEN);
            break;
			
        default:
		{
			char* invalidProtocol = "invalid address";
			buffer = malloc(strlen(invalidProtocol));
            strncpy(buffer, invalidProtocol, strlen(invalidProtocol));
            break;
		}
    }
	
    return buffer;
}
