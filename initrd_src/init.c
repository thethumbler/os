#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void main()
{
	if(!fork())
	{
		execve("/bin/login", NULL, NULL);
		open("/dev/tty/0", 0);
		write(0, "login not found\n", 17);
		_exit(0);
	}
	//uint64_t status;
	//wait(&status);
	for(;;);
}
