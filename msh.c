/*    $Id: $    */
/* CS 352 -- Mini Shell!  
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001 
 *   Modified January 6, 2003
 *
 */

/*
   Eric Ambrose
   February 14, 2014
   Assignment 4
   Msh.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proto.h"

/* Constants */

#define LINELEN 1024
#define EXPANDLEN 2000

// Global variable//

int ArgcG;
char** ArgvG;
int ShiftArgc;
int ShiftOffset;
int status;
int ShiftCheck;

/* Prototypes */

void processline (char *line);

/* Shell main */

int main (int mainargc, char **mainargv)
{
    char   buffer [LINELEN];
    int    len;
	char *mode = "r";
	int initialize = 1;
	
	//Initialize global argc and argv
	ArgcG = mainargc - 1; //not including script name
	ArgvG = mainargv; // arg[0] is shell name
	
	ShiftArgc = ArgcG;
	ShiftOffset = 0;
	
	//Check if there was an attempt to include a Script
	if ((mainargc > 1) && (initialize == 1)) {
		initialize = 0;
		stdin = fopen(mainargv[1], mode);
		if (stdin == NULL) {
			fprintf(stderr, "Can't open file provided\n");
			exit(127); //exit value given for no reachable file
		}
	}
	
    while (1) {

	//Only display %% if Interactive run
	//No Script or File provided.
        /* prompt and get line */
	if (mainargc ==  1){		
		ShiftCheck = 1;
		fprintf (stderr, "%% ");
	}
	
	if (fgets (buffer, LINELEN, stdin) != buffer)
	  break;

        /* Get rid of \n at end of buffer. */
	len = strlen(buffer);
	if (buffer[len-1] == '\n')
	    buffer[len-1] = 0;

	/* Run it ... */
	processline (buffer);

    }

	
    if (!feof(stdin))
        perror ("read");
		
	if (feof(stdin)){
		fclose(stdin);
		return 0;
	}
	
    return 0;		/* Also known as exit (0); */
}


void processline (char *line)
{
    pid_t  cpid;
	int argcount;
    char **argv;
	char expandedline[EXPANDLEN];
	
	if (expand(line, expandedline, EXPANDLEN)){
		return;
	}
	
    argcount = arg_parse(expandedline, &argv);

	
	//No arguments so no processing can be done
	if (argcount == 0){
		printf("No arguments were given\n");
		return;
	}
	
	if (builtin(argcount, argv)){
		return;
	}
	
    /* Start a new process to do the job. */
    cpid = fork();
    if (cpid < 0) {
      perror ("fork");
      return;
    }
    
    /* Check for who we are! */
    if (cpid == 0) {
      /* We are the child! */
      execvp (argv[0], argv);
      perror ("exec");
      exit (127);
    }
	
    /* Have the parent wait for child to complete */
    if (wait (&status) < 0)	
      perror ("wait");
	
    free(argv);
	
}















	
	
	



