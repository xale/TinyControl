// TCTypes.c
// Created by xale on 3/30/10
//
// Definitions of constants from TCTypes.h
//

#include "TCTypes.h"

const struct timeval TC_HANDSHAKE_TIMEOUT =	{5, 0}; // 5 seconds
const size_t TC_HANDSHAKE_BUFFER_SIZE =		16;
const char* TC_HANDSHAKE_SYN_MSG =			"TC_SYN";
const char* TC_HANDSHAKE_SYNACK_MSG =		"TC_SYNACK";
const char* TC_HANDSHAKE_ACK_MSG =			"TC_ACK";
