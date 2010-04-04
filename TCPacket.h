// TCPacket.h
// Created by xale on 3/20/10
//
// Header file with constants and definitions related to packets as specified by the TinyControl protocol.
//

#ifndef TCPACKET_H
#define TCPACKET_H

#include <stdint.h>

// All field widths are measured in bytes
#define SEQ_NUM_FIELD_WIDTH			(4)
#define TIMESTAMP_FIELD_WIDTH		(4)
#define RTT_FIELD_WIDTH				(4)
#define DATA_PACKET_HEADER_LENGTH	(SEQ_NUM_FIELD_WIDTH + TIMESTAMP_FIELD_WIDTH + RTT_FIELD_WIDTH)
#define MAX_PAYLOAD_SIZE			(1000)
#define DATA_PACKET_SIZE			(MAX_PAYLOAD_SIZE + DATA_PACKET_HEADER_LENGTH)
#define ELAPSED_T_FIELD_WIDTH		(4)
#define RECV_RATE_FIELD_WIDTH		(4)
#define LOSS_RATE_FIELD_WIDTH		(4)
#define FEEDBACK_PACKET_SIZE		(TIMESTAMP_FIELD_WIDTH + RECV_RATE_FIELD_WIDTH + LOSS_RATE_FIELD_WIDTH + ELAPSED_T_FIELD_WIDTH)

typedef uint8_t payload_t[MAX_PAYLOAD_SIZE];
typedef struct data_packet
{
	uint32_t seq_number;
	uint32_t timestamp;
	uint32_t rtt;
	payload_t payload;
} data_packet;
typedef struct feedback_packet
{
	uint32_t timestamp;
	uint32_t elapsed_time;
	uint32_t receive_rate;
	uint32_t loss_event_rate;
} feedback_packet;

#endif // TCPACKET_H
