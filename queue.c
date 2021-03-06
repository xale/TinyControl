#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

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
	pthread_mutex_init(&ret->empty_lock, NULL);
	pthread_cond_init(&ret->empty, NULL);
	return ret;
}

queue_elem* pop_front(queue* q)
{
	assert(q->head != NULL);
	queue_elem *ret = q->head;
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
	last->next = NULL;
	if (q->tail == NULL || q->head == NULL)
	{
		q->head = q->tail = last;
	}
	else
	{
		q->tail->next = last;
		q->tail = last;
	}
	q->count++;
	pthread_mutex_lock(&q->empty_lock);
	pthread_cond_signal(&q->empty);
	pthread_mutex_unlock(&q->empty_lock);
	return 0;
}

unsigned int queue_count(queue* q)
{
	return q->count;
}

void free_queue(queue* q)
{
	for(queue_elem *elem = q->head, *next; elem != NULL;)
	{
		next = elem->next;
		free(elem);
		elem = next;
	}

	free(q);
}
