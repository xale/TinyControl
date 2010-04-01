#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#include "queue.h"

queue* init_queue(void)
{
	queue* ret = malloc(sizeof(queue));
	if (ret == NULL)
	{
		return NULL;
	}
	ret->head = NULL;
	ret->tail = NULL;
	ret->count = 0;
	return ret;
}

queue_elem* pop_front(queue* q)
{
	assert(q->head != NULL);
	assert(q->count > 0);
	queue_elem* ret = q->head;
	q->head = ret->next;
	q->count--;
	return ret;
}

int push_back(queue* q, unsigned int payload_size, payload_t payload)
{
	queue_elem* last = malloc(sizeof(queue_elem));
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

void free_queue(struct queue* q)
{
	for(struct queue_elem *elem = q->head, *next; elem != NULL;)
	{
		next = elem->next;
		free(elem);
		elem = next;
	}

	free(q);
}
