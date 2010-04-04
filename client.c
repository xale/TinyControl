#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

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

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		return EXIT_FAILURE;
	}
	int sock = lookup(argv[1], argv[2]);
	queue *q = init_queue();

	pthread_t reader_tid;
	int status;
	struct reader_args args = { .sock = sock, .q = q };
	status = pthread_create(&reader_tid, NULL, reader_wrap, (void *) &args);
	// FIXME check status
	pthread_join(reader_tid, NULL);
	
	free_queue(q);
	return EXIT_SUCCESS;
}
