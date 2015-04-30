#include <stdio.h>

#include "umalloc.h"

int main(void)
{
	//printf("META_SIZE  == 0x%X\n",(unsigned) META_SIZE);
	//printf("sizeof(md) == 0x%X\n",(unsigned) sizeof(md));
	ufree(ucalloc(0,0));		//initializing just for testing
	void *temp1 = ucalloc(28,1);		//for testing
	void *temp2 = ucalloc(28,1);		//for testing
	//printf("%lX\n",(unsigned long)temp1);
	//printf("%lX\n",(unsigned long)temp2);
	ufree(temp1);
	ufree(temp2);

	printf("%lX\n",(unsigned long)ucalloc(10,1));
	printf("%lX\n",(unsigned long)ucalloc(1,1));
	printf("%lX\n",(unsigned long)umalloc(2));
	printf("%lX\n",(unsigned long)umalloc(3));
	return 0;
}

