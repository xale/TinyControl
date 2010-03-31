#ifndef QUEUE_H
#define QUEUE_H

#include "TCPacket.h"

typedef struct queue_elem
{
	struct queue_elem* next;
	unsigned int payload_size;
	payload_t payload;
} queue_elem;

typedef struct queue
{
	struct queue_elem* head;
	struct queue_elem* tail;
	unsigned int count;
} queue;

queue* init_queue(void);
queue_elem* pop_front(queue*);
int push_back(queue*, unsigned int, payload_t);
void free_queue(struct queue*);

#endif // QUEUE_H
