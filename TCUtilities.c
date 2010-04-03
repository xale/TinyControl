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
#include "TCUtilities.h"

int TCGetAddressInfo(const char* hostname, const char* port, int flags, inet_address_info* result)
{
	if (result == NULL)
		return -1;
	
	// Create the address lookup "hints": IPv4 only, UDP only
	inet_address_info addressHints;
	memset(&addressHints, 0, sizeof(addressHints));
	addressHints.ai_family = AF_INET;
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