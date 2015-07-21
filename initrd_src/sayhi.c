#include <stdio.h>

#define SETCOL(color)	\
	{ \
		char __buf__[7]; \
		sprintf(__buf__, "\033[%dm", color); \
		fprintf(stderr, __buf__);\
	}

int main(int argc, char **argv)
{
	if(argc != 2) return -1;

	SETCOL(46);
	SETCOL(30);

	fprintf(stderr, "Welcome %s, nice to see you ;)\n", argv[1]);
	
	SETCOL(0);
}
