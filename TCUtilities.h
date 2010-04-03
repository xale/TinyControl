/*
 *  TCUtilities.h
 *  TinyControl
 *
 *  Created by xale on 4/3/10.
 *
 */

#include "TCTypes.h"

// Stores the result of getaddrinfo() with the appropriate options in 'result'; returns 0 on success, -1 on failure
int TCGetAddressInfo(const char* hostname, const char* port, int flags, inet_address_info* result);
