#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "reader.h"
#include "queue.h"

struct reader_args
{
	int sock;
	queue *q;
};

void *reader_wrap(void *argv)
{
	struct reader_args *args = (struct reader_args *) argv;
	reader(args->sock, args->q);
	return NULL;
}

int writer(queue* q)
{
	queue_elem *pqe;
	ssize_t bytes_written;
	while (q->count > 0)
	{
		fprintf(stderr, "Writer's queue contains %d elements.\n", q->count);
		pqe = pop_front(q);
		fprintf(stderr, "Popped packet.\n");
		bytes_written = write(STDOUT_FILENO, pqe->payload, pqe->payload_size);
		// FIXME: check bytes_written
		free(pqe);
	}

	return 0;
}

struct writer_args
{
	queue *q;
	pthread_t reader_tid;
};

void *writer_wrap(void *argv)
{
	struct writer_args *args = (struct writer_args*) argv;
	int success = -1;
	while (success != 0)
	{
		writer(args->q);
		success = pthread_tryjoin_np(args->reader_tid, NULL);
		//fprintf(stderr, "Success in writer joining reader? %d.\n", success);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		return EXIT_FAILURE;
	}
	int sock = lookup(argv[1], argv[2]);
	queue *q = init_queue();

	pthread_t reader_tid, writer_tid;
	int status;
	struct reader_args r_args = { .sock = sock, .q = q };
	status = pthread_create(&reader_tid, NULL, reader_wrap, &r_args);
	fprintf(stderr, "Status of creating reader: %d.\n", status);
	// FIXME check status

	struct writer_args w_args = { .q = q, .reader_tid = reader_tid };
	status = pthread_create(&writer_tid, NULL, writer_wrap, &w_args);
	fprintf(stderr, "Status of creating writer: %d.\n", status);
	// FIXME check status

	pthread_join(writer_tid, NULL);
	
	free_queue(q);
	return EXIT_SUCCESS;
}
