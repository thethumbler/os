#include <system.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

sighandler_t *signal(int signum, sighandler_t handler)
{
	syscall(SYS_SIGNAL, signum, (uint64_t)handler, 0);
}

int kill(int pid, int signum)
{
	syscall(SYS_KILL, pid, signum, 0);
}

int raise(int signum)
{
	kill(getpid(), signum);
}
