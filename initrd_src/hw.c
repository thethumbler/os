#include <system.h>
#include <fcntl.h>
#include <unistd.h>

void _start()
{
	write(0, "Hello, World!\n", 14);
	_exit(0);
	for(;;);
}
