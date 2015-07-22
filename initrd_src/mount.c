#include <stdio.h>
#include <sys/mount.h>

int main(int argc, char **argv)
{
	if(argc < 3 || argc > 4)
	{
		printf("usage: %s fs [src] dst\n", argv[0]);
		return -1;
	}
	
	char *fs  = argv[1];
	char *dst = argc == 4 ? argv[3] : argv[2];
	char *src = argc == 4 ? argv[2] : NULL;
	
	switch(mount(src, dst, fs, 0, NULL))
	{
		case -2:
			printf("unkown filesystem: %s\n", fs);
			return -2;
		case -3:
			printf("destination not found: %s\n", dst);
			return -3;
	}
}
