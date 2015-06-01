#include <system.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void _start()
{	
	while(1)
	{
		write(0, "Hello, World!\n", 14);
		long x = 5000000;
		while(--x);
	}
}
