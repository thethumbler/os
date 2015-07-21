#include <stdio.h>

int main()
{
	open("/dev/tty/0", 0);
	open("/dev/tty/0", 0);
	open("/dev/tty/0", 0);
	
	char usr[80];
	while(!usr[0] || usr[0] == '\n') 
	{
		fprintf(stderr, "username : ");
		read(0, usr, 80);
	}
	
	int i;
	for(i = 0; usr[i] != '\n'; ++i);
	usr[i] = 0;
	
	char _usr[80];
	strcat(_usr, "USER=");
	strcat(_usr, usr);
	
	char *env[] = { _usr, "PWD=/", NULL };
	execve("/bin/shell", NULL, env);
}
