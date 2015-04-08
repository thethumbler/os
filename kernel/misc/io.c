#include <system.h>
#include <io.h>

inline uint8_t inb(uint32_t port)
{
	uint8_t ret;
	asm volatile ("inb %w1, %b0" : "=a"(ret) : "d"(port));
	return ret;
}

inline void outb(uint32_t port, uint8_t val)
{
	asm volatile ("outb %b0, %w1" : /* No Output */ : "a"(val), "d"(port));
}

