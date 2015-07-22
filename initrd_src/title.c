#include <stdio.h>
#include <string.h>

#define WIDTH	80
#define HEIGHT	25
#define POS(str) ((WIDTH - strlen(str))/2)

#define SETCUR(r, c) printf("\033[%d;%dH", r, c);
#define SETCOL(col)	printf("\033[%dm", col);
int main(int argc, char **argv)
{
	if(argc != 2) return -1;

		
	SETCUR(1, 1)
	SETCOL(42);
	SETCOL(30);
	
	int i;
	for(i = 0; i < WIDTH; ++i)
		printf(" ");

	SETCUR(1, POS(argv[1]));
	printf("[%s]", argv[1]);
	SETCOL(0);
	SETCUR(2, 1);
}
