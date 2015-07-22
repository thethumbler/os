#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if(argc < 2 || argc > 3)
	{
		printf("usage: %s filename [count]\n", argv[0]);
		return -1;
	}
	
	FILE *file = fopen(argv[1], "r");
	//fseek(file, 1024*1024, SEEK_SET);
	
	unsigned i, count = argc == 3 ? atoi(argv[2]) : 1024;
	for(i = 0; i < count; ++i)
	{
		unsigned short sig;
		fread((void*)&sig, 2, 1, file);
		if(sig == 0xEF53)
			printf("%u: %X\n", i/512, sig);
	}
}
