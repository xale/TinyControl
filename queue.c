#include <stdlib.h>
#include <types.h>
#include <stdint.h>
#include <assert.h>

#include "queue.h"

struct queue* init_queue(void)
{
	struct queue* ret = malloc(sizeof(struct queue));
	ret->head = NULL;
	ret->tail = NULL;
	ret->size = 0;
	return ret;
}

struct queue_elem* pop_front(struct queue* q)
{
	assert(q->head != NULL);
	assert(q->size > 0);
	struct queue* ret = q->head;
	q->head = ret->next;
	q->size--;
	return ret;
}

int push_back(struct queue* q, int payload_size, payload_t payload)
{
	struct queue* last = malloc(sizeof(queue_elem));
	if (last == NULL)
	{
		return -1;
	}
	last->payload_size = payload_size;
	last->payload = payload;
	q->tail->next = last;
	q->tail = last;
	q->size++;
	return 0;
}
