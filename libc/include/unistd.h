#ifndef _UNISTD_H
#define _UNISTD_H

#include <system.h>

void _exit(uint64_t status);
uint64_t close(uint64_t fd);
uint64_t read(uint64_t fd, void *buf, uint64_t count);
uint64_t write(uint64_t fd, void *buf, uint64_t count);
uint64_t fork(void);
uint64_t execve(uint8_t *path, uint8_t **arg, uint8_t **env);
void *sbrk(uint64_t size);
uint64_t getpid();
uint64_t wait(uint64_t *status);
uint64_t usleep(uint64_t usec);

#endif
