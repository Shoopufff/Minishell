#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


int main (void) {

	char str[15] = {0};

	char *end = str;
	end += sprintf(end, "%s ", "hello!");
	end += sprintf(end, "%ld", (long)getpid());
	
	return 0;
}