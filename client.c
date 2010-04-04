#include <stdlib.h>
#include <stdio.h>

#include "reader.h"
#include "queue.h"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		return EXIT_FAILURE;
	}
	int sock = lookup(argv[1], argv[2]);
	queue *q = init_queue();

	reader(sock, q);
	
	free_queue(q);
	return EXIT_SUCCESS;
}
