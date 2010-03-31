#include <stdlib.h>
#include <types.h>
#include <stdint.h>
#include <assert.h>

#include "queue.h"

queue* init_queue(void)
{
	queue* ret = malloc(sizeof(queue));
	ret->head = NULL;
	ret->tail = NULL;
	ret->size = 0;
	return ret;
}

queue_elem* pop_front(queue* q)
{
	assert(q->head != NULL);
	assert(q->size > 0);
	queue* ret = q->head;
	q->head = ret->next;
	q->size--;
	return ret;
}

int push_back(queue* q, int payload_size, payload_t payload)
{
	queue* last = malloc(sizeof(queue_elem));
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
