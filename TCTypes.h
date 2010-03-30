// TCTypes.h
// Created by xale on 3/30/10
//
// Header file with typedefs and defines for TinyControl protocol.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#ifndef TCTYPES_H
#define TCTYPES_H

typedef struct addrinfo				inet_address_info;
typedef struct sockaddr_in			inet_socket_address;
typedef int 						socket_fd;

extern const struct timeval TC_HANDSHAKE_TIMEOUT;
extern const size_t TC_HANDSHAKE_BUFFER_SIZE;
extern const char* TC_HANDSHAKE_SYN_MSG;
extern const char* TC_HANDSHAKE_SYNACK_MSG;
extern const char* TC_HANDSHAKE_ACK_MSG;

#endif // TCTYPES_H
