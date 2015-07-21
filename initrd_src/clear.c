#include <stdio.h>

int main()
{
	fprintf(stderr, "\033[H\033[2J");
	return 0;
}
