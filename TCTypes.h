// TCTypes.h
// Created by xale on 3/30/10
//
// Header file with typedefs and defines for TinyControl protocol.
//

#include <netinet/in.h>

#ifndef TCTYPES_H
#define TCTYPES_H

typedef struct addrinfo				inet_address_info;
typedef struct sockaddr_in			inet_socket_address;
typedef int 						socket_fd;

const struct timeval TC_HANDSHAKE_TIMEOUT = {5, 0}; // 5 seconds
const size_t TC_HANDSHAKE_BUFFER_SIZE =		16;
const char* TC_HANDSHAKE_SYN_MSG =			"TC_SYN";
const char* TC_HANDSHAKE_SYNACK_MSG =		"TC_SYNACK";
const char* TC_HANDSHAKE_ACK_MSG =			"TC_ACK";

#endif // TCTYPES_H
