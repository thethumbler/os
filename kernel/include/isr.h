#ifndef _ISR_H
#define _ISR_H

#include <system.h>

typedef struct
{
	uint64_t rdi, rsi, rbp, rsp, ebx, ecx, edx, eax;
	uint64_t rip, cs, eflags, _rsp, ss;
} __attribute__((packed)) regs_t;

#endif
