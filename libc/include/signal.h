#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef void(*sighandler_t)(int);
typedef enum 
{
	SIGHUP 	= 1,
	SIGINT	= 2,
	SIGQUIT	= 3,
	SIGILL	= 4,
	SIGTRAP	= 5,
	SIGABRT	= 6,
	SIGBUS	= 7,
	SIGFPE	= 8,
	SIGKILL	= 9,
	SIGUSR1	= 10,
	SIGSEGV	= 11,
	SIGUSR2	= 12,
	SIGPIPE	= 13,
	SIGALRM	= 14,
	__UNUSED__,
	SIGCHLD	= 16,
	SIGCONT	= 17,
	SIGSTOP	= 18,
	SIGTSTP	= 19,
	SIGTTIN	= 20,
	SIGTTOU	= 21,
} signal_num_t;

sighandler_t *signal(int signum, sighandler_t handler);
int kill(int pid, int signum);
int raise(int signum);

#endif
