#include <system.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

void _start()
{
	int fd = open("/hd/MyFile", 0);
	char buf[30];
	read(fd, buf, 30);
	write(0, buf, 30);
	write(0, "\n", 1);
	_exit(0);
}
