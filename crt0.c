#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

//extern void exit(int code);
extern int main (int argc, char **argv);

#define syscall2(call, arg0, arg1) \
	asm("int $0x80;":: "a"(call), "b"(arg0), "c"(arg1));
	
int getatt(int req, char *buf)
{
	syscall2(13, req, buf);
}
	
void _start() {	
	int argslen = getatt(0, NULL);
	char *args = malloc(argslen + 1);
	memset(args, 0, argslen);
	getatt(1, args);
	
	int i, argc = 0, envc = 0;
	char *_ = args;
	while(*_)
	{
		++argc;
		_ = _ + strlen(_) + 1;
	}
	
	char *argv[argc + 1];
	char *arg = args;
	for(i = 0; i < argc; ++i)
	{
		argv[i] = arg;
		arg += strlen(arg) + 1;
	}
	
	argv[argc] = NULL;
	
	int envslen = getatt(2, NULL);
	char *envs = malloc(envslen + 1);
	memset(envs, 0, envslen);
	getatt(3, envs);
	
	_ = envs;
	while(*_)
	{
		++envc;
		_ = _ + strlen(_) + 1;
	}
	
	char *envv[envc + 1];
	char *env = envs;

	for(i = 0; i < envc; ++i)
	{
		envv[i] = env;
		env += strlen(env) + 1;
	}
	
	envv[envc] = NULL;
	
	extern char **environ;
	environ = envv;
	
	//_init_signal();
    int ex = main(argc, argv);
    exit(ex);
}
