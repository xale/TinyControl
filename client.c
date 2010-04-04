#include <stdlib.h>
#include <stdio.h>
#include "reader.h"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		return EXIT_FAILURE;
	}
	lookup(argv[1], argv[2]);
	
	return EXIT_SUCCESS;
}
