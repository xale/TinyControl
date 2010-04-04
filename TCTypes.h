// TCTypes.h
// Created by xale on 3/30/10
//
// Header file with typedefs and defines for TinyControl protocol.
//

#ifndef TCTYPES_H
#define TCTYPES_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

typedef struct addrinfo				inet_address_info;
typedef struct addrinfo*			inet_address_info_list;
typedef struct sockaddr				socket_address;
typedef struct sockaddr_in			inet_socket_address;
typedef struct sockaddr_in6			inet6_socket_address;
typedef socklen_t					socket_address_length;
typedef int 						socket_fd;
typedef struct timeval				time_of_day;
typedef struct timeval				time_delta;
typedef int							file_fd;

extern const time_delta TC_HANDSHAKE_TIMEOUT;
extern const size_t TC_HANDSHAKE_BUFFER_SIZE;
extern const char* TC_HANDSHAKE_SYN_MSG;
extern const char* TC_HANDSHAKE_SYNACK_MSG;
extern const char* TC_HANDSHAKE_ACK_MSG;

#endif // TCTYPES_H
