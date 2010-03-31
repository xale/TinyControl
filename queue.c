#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#include "queue.h"

struct queue* init_queue(void)
{
	struct queue* ret = malloc(sizeof(struct queue));
	ret->head = NULL;
	ret->tail = NULL;
	ret->count = 0;
	return ret;
}

struct queue_elem* pop_front(struct queue* q)
{
	assert(q->head != NULL);
	assert(q->count > 0);
	struct queue_elem* ret = q->head;
	q->head = ret->next;
	q->count--;
	return ret;
}

int push_back(struct queue* q, unsigned int payload_size, payload_t payload)
{
	struct queue_elem* last = malloc(sizeof(struct queue_elem));
	if (last == NULL)
	{
		return -1;
	}
	last->payload_size = payload_size;
	memcpy(last->payload, payload, payload_size);
	q->tail->next = last;
	q->tail = last;
	q->count++;
	return 0;
}
