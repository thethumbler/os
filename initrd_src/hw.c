#include <system.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void debugx(int);

void _start()
{	
	open(-1, 0);
	
	write(0, "Hello, World!\n", 14);
	_exit(0);
	for(;;);
}


void debugx(int val)
{
	uint8_t *enc = "0123456789ABCDEF";
	uint8_t buf[10];
	buf[8] = '\n';
	buf[9] = '\0';
	uint8_t i = 8;
	while(i)
	{
		buf[--i] = enc[val&0xF];
		val >>= 4;
	}
	write(0, buf, 10);
}
