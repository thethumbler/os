#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("usage: %s tests_count\n", argv[0]);
		return -1;
	}

	int tests = atoi(argv[1]), i;
	for(i = 0; i < tests; ++i)
	{
		int a;
		if(a = fork())
		{
			fprintf(stderr, "spawned child %d\n", a);
		}
		else
			for(;;);
	}
}
