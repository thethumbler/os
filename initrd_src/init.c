#include <system.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void _start()
{	
	
	if(!fork())
	{
		execv("/shell", 0);
		_exit(0);
	}
	//uint64_t status;
	//wait(&status);
	for(;;);
}
