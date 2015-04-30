#include <system.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

void _start()
{
	open("/dev/tty0", 0);
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
				execv(buf, 0);
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
