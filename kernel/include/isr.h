#ifndef _ISR_H
#define _ISR_H

#include <system.h>

typedef struct
{
	uint64_t 
		r15, r14, r13, r12, r11, r10, r9, r8,
		rdi, rsi, rbp, rbx, rcx, rdx, rax,
		rip, cs, rflags, rsp, ss;
} __attribute__((packed)) regs_t;

#endif
