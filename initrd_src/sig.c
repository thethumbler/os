#include <stdio.h>
#include <signal.h>

void handler(int _)
{
	printf("in signal\n");
}

int main()
{
	printf("Registering handler\n");
	signal(SIGUSR1, handler);
	raise(SIGUSR1);
}

