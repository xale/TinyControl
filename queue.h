#ifndef QUEUE_H
#define QUEUE_H

#include "TCPacket.h"
#include "pthread.h"

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
	pthread_mutex_t empty_lock;
	pthread_cond_t empty;
} queue;

queue* init_queue(void);
queue_elem* pop_front(queue*);
int push_back(queue*, unsigned int, payload_t);
unsigned int queue_count(queue*);
void free_queue(queue*);

#endif // QUEUE_H
