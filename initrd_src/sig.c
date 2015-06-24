#include <unistd.h>
#include <signal.h>
#include <stdint.h>

void handler(int);

void _start()
{
	write(0, "Registering handler\n", 21);
	signal(SIGUSR1, handler);
	raise(SIGUSR1);
	_exit(0);
	for(;;);
}

void handler(int _)
{
	write(0, "in signal\n", 11);
}
