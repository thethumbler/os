#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	signal(SIGINT, SIG_IGN);
	
	char buf[100];
	chdir("/");
	while(1)
	{
		fprintf(stderr, "[ %s : %s ] # ", getenv("USER"), getcwd(NULL, 50));
		memset(buf, 0, 100);
		read(0, buf, 100);
	
		char *tokens[20], *p = buf;
		
		int i = 0;
		tokens[0] = p;
		
		if(*p == '\n') continue;
		while(*p != '\n' && i < 20)
		{
			if(*p == ' ')
			{
				*p = '\0';
				++p;
				
				if(*p == '\"')	// Quotes protected argument.. let's take them out
				{
					*p = '\0';
					tokens[++i] = ++p;
					--p;
					while(*++p && *p != '\"');
					*p = '\0';
					++p;
					continue;
				}
				
				if(*p != '\n')
					tokens[++i] = p;
			}
			else
				++p;
		}

		tokens[++i] = NULL;
		
		if(*p == '\n') { *p = '\0'; *++p = '\0'; }
		
		if(!strcmp(tokens[0], "cd"))	// Change directory
		{
			if(tokens[1][0] == '/')	// Canonical path
			{
				chdir(tokens[1]);
			} else
			{
				char *cwd = getcwd(NULL, 50);
				char *path = malloc(strlen(cwd) + strlen(tokens[1]) + 1);
				strcat(path, cwd);
				strcat(path + strlen(cwd), tokens[1]);
				free(cwd);
				chdir(path);
				free(path);
			}
			continue;
		}
		
		const char *bin = "/bin/";
		
		if(!fork())	// in Child
		{
			// Let's tokenize the buffer
			char _pname[strlen(bin) + strlen(tokens[0]) + 1];
			memset(_pname, 0, sizeof(_pname));
			strcat(_pname, bin);
			strcat(_pname + strlen(bin), tokens[0]);
			
			char *pname = _pname;
			
			extern char **environ;
			execve(pname, &tokens[1], environ);
			
			fprintf(stderr, "%s : command not found\n", tokens[0]);
			exit(0);
			for(;;);	// We should never reach this anyway
		}
		int stat;
		wait(&stat);
	}
}
