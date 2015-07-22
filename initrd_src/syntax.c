#include <stdio.h>
#include <string.h>

#define SETCOL(col) \
	{ char ___buf[6]; sprintf(___buf, "\033[%dm", col); fprintf(stderr, ___buf); }

char color[] = { 36, 35, 33, 92 };

char *kwrds[] = {	"auto", "char", "const", "double", "extern", 
				 	"float", "int", "long", "register", "short", 
				 	"signed", "static", "unsigned", "void", "volatile", "\0",

					"sizeof", "\0"

					"break", "case", "continue", "default", "do", 
					"else", "enum", "for", "goto", "if", "return", 
					"struct", "switch", "typedef", "union", "while", "\0",
					
					"#include", "\0",
					NULL
				};

int within(char chr, char *str)
{
	while(*str)
		if(chr == *str++)
			return 1;
	return 0;
}
					
int main(int argc, char **argv)
{
	if(argc != 2) return -1;
	
	FILE *file = fopen(argv[1], "r");
	fseek(file, 0, SEEK_END);
	unsigned size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char _buf[size];
	fread(_buf, size, 1, file);
	
	char *pos = _buf, *buf = _buf;
	char br;
	
	while(*buf)
	{
		while(*buf && !within(*buf, " \n\t;\"")) ++buf;	// Breaks
		
		br = *buf;
		
		*buf = '\0';
		
		int color_n = 0;
		char **kwrd = kwrds;
		while(*kwrd)
		{
			if(!**kwrd) { ++color_n; ++kwrd; continue; }
			if(!strcmp(pos, *kwrd++))
				SETCOL(color[color_n]);
		}
		
		fprintf(stderr, "%s\033[0m", pos == buf ? "" : pos);
		
		if(br == '\"')
		{
			fprintf(stderr, "\033[32m\"");
			pos = ++buf;
			
			while(*buf != '\"') ++buf;
			*buf = '\0';
			fprintf(stderr, "%s\"\033[0m", pos);
			pos = ++buf;
			continue;
		}
		
		fprintf(stderr, "%c", br);
		
		pos = ++buf;
	}
	fprintf(stderr, "\n");
}
