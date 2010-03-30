#ifndef QUEUE_H
#define QUEUE_H

#include "TCPacket.h"

typedef payload uint8_t payload[MAX_DATA_PAYLOAD_SIZE];

struct queue_elem
{
	struct queue_elem* next;
	int payload_size;
	payload payload;
};

struct queue
{
	struct queue_elem* head;
	struct queue_elem* tail;
};

struct queue* init_queue(void);
struct queue_elem* pop_front(struct queue*);
int push_back(struct queue*);


#endif // QUEUE_H
