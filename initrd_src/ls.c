#include <stdio.h>
#include <unistd.h>
#include <bits/dirent.h>

int main(int argc, char **argv)
{
	char *cwd;
	if(argc != 2)
		cwd = getcwd(NULL, 50);
	else
		cwd = argv[1];
	
	DIR *dir = opendir(cwd);
	struct dirent *ent = NULL;
	while(ent = readdir(dir))
		printf("%s\n", ent->d_name);
}
