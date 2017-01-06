/* CS 352 -- Mini Shell!  
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001 
 *   Modified January 6, 2003
 *
 */

/*
   Eric Ambrose
   January 10, 2014
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

//#include 'proto.h'

/* Constants */ 

#define LINELEN 1024

/* Prototypes */

void processline (char *line);

char **arg_parse (char *line);

/* Shell main */

int
main (void)
{
    char   buffer [LINELEN];
    int    len;

    while (1) {

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
    int    status;
    char **argv;    

    argv = arg_parse(line);

	//printf("Processline says this is argv[0] %s\n", argv[0]);
	
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

char **arg_parse (char *line)
{
	char **Mpointer;
	char *ptr;
	int argcounter = 0;
	int count = 0;
	int slength;
	//int sizeofarg = 0;
	//int currentarg = -1;
	int leadingspaces = 0;
	int test = 0;
	int i; //index value counter
	int ProcessingArg; //Use as boolean for if arguments are being processed  
	

	ProcessingArg = 0; //initialize to 0, so it represents "false"
	argcounter = 0;
	slength = strlen(line);
	ptr = line;
	
	//count the number of leading spaces as to ignore them when counting args
	for (i=0; i < slength; i++, ptr++){
		if (line[i] != ' '){
			break;
		}
		else if  (line[i] == ' ')
			leadingspaces++;
	}

	//testing lead space count
	//printf("Leading Spaces: %d\n",leadingspaces);

	//Must count arguments first in order to correctly Malloc the pointer array
	for ( i=leadingspaces; i < slength; i++){	
		if ( (line[i] == ' ') && (ProcessingArg == 0) ){ //skip the extra spaces
			continue;
		}
	 	else if ( (line[i] == ' ') && (ProcessingArg == 1) ){ //this space symbolizes end of arg
			ProcessingArg = 0;			
		}
		else if ( (line[i] != ' ') && (ProcessingArg == 1) ){ //current argument processing			
			continue;
		}
		else if (line[i] != ' '){
			ProcessingArg = 1; //Index of beginning of argument found			
			argcounter = argcounter + 1;
		}
	}
	
	//testing arg count
	//printf("Arg count: %d\n", argcounter);

	//add extra spot for Null Pointer
	ProcessingArg = 0;
	Mpointer = (char **)malloc(sizeof(char*)*(argcounter + 1)); 

	if (argcounter == 0){
		printf("no arguments evaluated\n");
		return Mpointer;
	}
	else{
	
	//Get the beginning of each argument to put into the array that we malloc'd
	for ( i=leadingspaces; i < slength; i++, ptr++){
		if ( (line[i] != ' ') && (ProcessingArg == 0) ){  //Index of beginning of argument
			ProcessingArg = 1;
			//currentarg = currentarg + 1;
			//sizeofarg = sizeofarg + 1;
			Mpointer[count] = ptr;  //Set array at count to point to character current spot in line
			//printf("This is the start of the current Argument: %c\n", line[i]);			
			count = count +1;
		}
		else if ( (line[i] == ' ') && (ProcessingArg == 1) ){ //this space symbolizes end of arg
			/*printf("Current arg: %d  Size of arg: %d  Arg: ", currentarg, sizeofarg);
			for (test=0; test < sizeofarg; test++) {
				printf("%c", line[i-sizeofarg+test]);
			}
			printf("\n");
			sizeofarg = 0;*/
			ProcessingArg = 0;
			line[i] = '\0';
		}
		else if ( (line[i] == ' ') && (ProcessingArg == 0) ){ //skip the extra spaces
			continue;		
		}
		else if ( (line[i] != ' ') && (ProcessingArg == 1) ){ //current argument processing
			//sizeofarg = sizeofarg + 1;
		}
	}
	}

	/*printf("Current arg: %d  Size of arg: %d  Arg: ", currentarg, sizeofarg);
	for (test=0; test < sizeofarg; test++) {
		printf("%c", line[i-sizeofarg+test]);
	}
	printf("\n");*/

	
	Mpointer[count] = '\0'; //Adding null pointer to end of the array
	return Mpointer;

}













	
	
	



