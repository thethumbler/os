#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <system.h>
//#include <process.h>

//Ignore
#define IGN	0
//Terminate
#define TRM	1
//Stop
#define STP	2
//Continue
#define CNT	3

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

extern int signal_default_action[];
//extern char *signal_names[];

#endif
