#ifndef _SYSCALL_H
#define _SYSCALL_H

#define SYS(f) void f(uint64_t rbx, uint64_t rcx, uint64_t rdx)

extern void* sys_calls[];

#define sys_calls_count sizeof(sys_calls)/0x8

#endif
