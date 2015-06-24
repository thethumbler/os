#include <system.h>
#include <stdint.h>

void _exit(uint64_t status)
{
	syscall(SYS_EXIT, status, 0 , 0);
}

uint64_t close(uint64_t fd)
{
	return syscall(SYS_CLOSE, fd, 0, 0);
}

uint64_t read(uint64_t fd, void *buf, uint64_t count)
{
	return syscall(SYS_READ, fd, (uint64_t)buf, count);
}

uint64_t write(uint64_t fd, void *buf, uint64_t count)
{
	return syscall(SYS_WRITE, fd, (uint64_t)buf, count);
}

uint64_t fork(void)
{
	return syscall(SYS_FORK, 0, 0, 0);
}

uint64_t execv(uint8_t *path, uint8_t **arg)
{
	return syscall(SYS_EXECV, (uint64_t)path, 0, 0);
}

void *sbrk(uint64_t size)
{
	return (void*)syscall(SYS_SBRK, size, 0, 0);
}

uint64_t getpid()
{
	return syscall(SYS_GETPID, 0, 0, 0);
}

uint64_t wait(uint64_t *status)
{
	return syscall(SYS_WAIT, (uint64_t)status, 0, 0);
}
