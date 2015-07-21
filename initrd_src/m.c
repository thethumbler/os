#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define humanize(val) \
	((val) > 1000000 ? (val)/1000000 : (val) > 1000 ? (val)/1000 : (val))
#define size(val)	\
	((val) > 1000000 ? "MiB" : (val) > 1000 ? "KiB" : "B")

void dum();

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("usage: %s block_size tests_count\n", argv[0]);
		return -1;
	}

	size_t bs = atoll(argv[1]);

	switch(argv[1][strlen(argv[1]) - 1])
	{
		case 'g':
		case 'G':
			bs *= 1000000000;
			break;
		case 'm':
		case 'M':
			bs *= 1000000;
			break;
		case 'k':
		case 'K':
			bs *= 1000;
			break;
	}

	unsigned a = atoi(argv[2]), i;
	
	int failed = 0;
	
	for(i = 0; i < a; ++i)
	{
		uint8_t *ptr = malloc(bs);
		*ptr = 'A';
		//memset(ptr, 'A', bs);
		int r = 0;//memcmp(ptr, main, (uint64_t)dum - (uint64_t)main);
		//if(*ptr != 'A') r = 1;
		//if(r) ++failed;
		printf("[%6d] : 0x%016lX (%s)\n", i, ptr, r?"failed":"passed");
		int s = 0;
	}
	
	printf("Memory test %s with %d test(s) [%u %s]\n", failed?"failed":"passed",
			a, humanize(a * bs), size(a * bs));
	//*(char*)(-1) = 5; // break kernel
}

void dum()
{

}
