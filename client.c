#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
		return -1;
	}
	lookup(argv[1], argv[2]);
}
