#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

void main()
{
	if(fork()) _exit(0);
	
	close(0);
	close(1);
	close(2);
	open("/dev/tty1", 0);
	open("/dev/tty1", 0);
	open("/dev/tty1", 0);
	
	char buf[100];
	while(1)
	{
		write(0, "Shell # ", 8);
		read(0, buf, 100);
		if(*buf == '/')	// Trying to start an application
		{
			if(!fork())	// in Child
			{
				// Let's take out the newline character
				uint64_t i;
				for(i = 0; i < 100; ++i) 
					if(buf[i] == '\n')
					{
						buf[i] = '\0';
						break;
					}
				execve(buf, 0, 0);
				for(;;);	// We should never reach this anyway
			}
			uint64_t stat;
			wait(&stat);
		}
		else
		write(0, buf, 100);
	}
	for(;;);
}
