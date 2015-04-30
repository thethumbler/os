#include <system.h>
#include <stdint.h>
#include <fcntl.h>

uint64_t open(const char *path, uint64_t flags)
{
	return syscall(SYS_OPEN, (uint64_t)path, flags, 0);
}
