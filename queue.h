#ifndef QUEUE_H
#define QUEUE_H

#include "TCPacket.h"

struct queue_elem
{
	struct queue_elem* next;
	unsigned int payload_size;
	payload_t payload;
};

struct queue
{
	struct queue_elem* head;
	struct queue_elem* tail;
};

struct queue* init_queue(void);
struct queue_elem* pop_front(struct queue*);
int push_back(struct queue*, unsigned int, payload_t);

#endif // QUEUE_H
