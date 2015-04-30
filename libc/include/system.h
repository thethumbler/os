#ifndef _SYSTEM_H
#define _SYSTEM_H

#undef __STDC_HOSTED__
#include <stdint.h>

#define SYS_EXIT	0
#define SYS_OPEN	1
#define SYS_READ	2
#define SYS_WRITE	3
#define SYS_CLOSE	4
#define SYS_FORK	5
#define SYS_EXECV	6
#define SYS_SBRK	7
#define SYS_GETPID	8
#define SYS_WAIT	9

uint64_t syscall(uint64_t rax, uint64_t rbx, uint64_t rcx, uint64_t rdx);

#endif
