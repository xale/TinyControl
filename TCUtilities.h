/*
 *  TCUtilities.h
 *  TinyControl
 *
 *  Created by xale on 4/3/10.
 *
 */

#include "TCTypes.h"
#include "TCPacket.h"

struct data_packet;

// Stores the result of getaddrinfo() with the appropriate options in 'result'; returns 0 on success, -1 on failure
int TCGetAddressInfo(const char* hostname, const char* port, int flags, inet_address_info* result);

// Returns a printable string in the form <IP address>:<port> for the specified socket address
char* TCPrintAddress(socket_address* address);

// Returns a string representation of the numeric address of the specified socket address
char* TCAddressToString(socket_address* address);

// Returns the port number of the specified socket address
uint16_t TCAddressGetPort(socket_address* address);

// Returns a printable string containing the information in the specified feedback packet
char* TCPrintFeedbackPacket(feedback_packet* packet);

void print_data_packet(struct data_packet *);

// Subtract time 'y' from time 'x', storing the result in 'result'. Returns 1 if the difference is negative, otherwise 0.
int time_subtract(time_delta* result, time_of_day* x, time_of_day* y);

// Converts a struct timeval (containing seconds and microseconds) to a number of milliseconds
uint32_t time_to_milliseconds(struct timeval* time);

uint32_t get_time_in_milliseconds();
