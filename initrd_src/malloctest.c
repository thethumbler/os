#include <stdio.h>
#include <stdlib.h>

void dum();
int main(int argc, char **argv)
{
	if(argc < 2 || argc > 4)
	{
		printf("usage: %s size [count] [skip_free]\n", argv[0]);
		exit(0);
	}
	
	int size = atoi(argv[1]);
	
	switch(argv[1][strlen(argv[1]) - 1])
	{
		case 'g':
		case 'G':
			size *= 1000000000;
			break;
		case 'm':
		case 'M':
			size *= 1000000;
			break;
		case 'k':
		case 'K':
			size *= 1000;
			break;
	}
	
	int count = argc >= 3 ? atoi(argv[2]) : 1;
	int skip_free = argc >= 4 ? atoi(argv[3]) : 1;
	int i = 0, failed = 0, fail = 0;
	while(count--)
	{
		void *ptr = malloc(size);
		//if(count || i) fprintf(stderr, "[%04d]: ", i++);
		//fprintf(stderr, "malloc(%d) = 0x%016lX ", size, ptr);
		int msize = (int)dum - (int)main;
		memcpy(ptr, main, msize);
		if(memcmp(ptr, main, msize)) ++failed;
		/*
		int j;
		for(j = 0; j < size; ++j)
			if(*(char*)(ptr+j) != 'A')
		*/
				
		//printf("(%s)\n", failed ? "failed" : "passed");
		fail += failed;
		failed = 0;
		if(!(++i%skip_free)) free(ptr);
	}
	fprintf(stderr, "malloc: %d test(s) %s\n", i, fail ? "failed":"passed");
	//*(char*)(-1) = 'A';
	_exit(0);
}

void dum()
{

}
