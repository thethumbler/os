#include <system.h>
#include <stdint.h>

uint64_t syscall(uint64_t rax, uint64_t rbx, uint64_t rcx, uint64_t rdx)
{
	asm("int $0x80;"
		:
		: "a"(rax), "b"(rbx), "c"(rcx), "d"(rdx)
		);
}
