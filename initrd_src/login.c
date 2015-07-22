#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define WIDTH	80
#define HEIGHT	25
#define POS(str) ((WIDTH - strlen(str))/2)

#define SETCUR(r, c) \
	{ \
		char __buf__[9]; \
		sprintf(__buf__, "\033[%d;%dH", r, c); \
		fprintf(stderr, __buf__);\
	}

#define SETCOL(color)	\
	{ \
		char __buf__[7]; \
		sprintf(__buf__, "\033[%dm", color); \
		fprintf(stderr, __buf__);\
	}

int main()
{
	open("/dev/tty/0", 0);
	open("/dev/tty/0", 0);
	open("/dev/tty/0", 0);
	
	char *welcome_msg = 
"=======================[ Welcome to the yet un-named OS ]=======================";
	SETCOL(36);
	fprintf(stderr, "\n\n%s\n", welcome_msg);
	SETCOL(0);
	
	fprintf(stderr, "Currently any user name shall work ;)\n");
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
