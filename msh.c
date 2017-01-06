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
   January 22, 2014
   Assignment 2
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


/* Prototypes */

void processline (char *line);


/* Shell main */

int
main (void)
{
    char   buffer [LINELEN];
    int    len;

    while (1) {
	
	printf("\\\n");

        /* prompt and get line */
	fprintf (stderr, "%% ");
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

    return 0;		/* Also known as exit (0); */
}


void processline (char *line)
{
    pid_t  cpid;
    int status;
	int argcount;
    char **argv;
	//int i;

    argcount = arg_parse(line, &argv);

	/*for (i=0; i<argcount; i++){
		printf("Processline says this is argv[%d] %s\n", i, argv[i]);
	}*/
	
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

    free(argv);
    
    /* Have the parent wait for child to complete */
    if (wait (&status) < 0)
      perror ("wait");
}















	
	
	



