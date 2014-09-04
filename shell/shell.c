#include <stdio.h>

int main(int argc, char **argv)
{
#ifdef SHELL_DEBUG
	int i = 0;
	printf("argc: %d\n", argc);
	for (i=0; i<argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);
#else
	printf("Hello!\n");
#endif
	return 0;
}
