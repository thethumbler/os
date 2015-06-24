#ifndef _TYPES_H
#define _TYPES_H

typedef uint32_t pid_t;

struct sigaction
{
	void (*sa_handler)(int);
	//void (*sa_sigaction)(int, siginfo_t *, void *);
	//sigset_t sa_mask;
	int sa_flags;
};

#endif
