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
   March 14, 2014
   Assignment 6
   Msh.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>

#include "proto.h"

/* Constants */

#define LINELEN 1024
#define EXPANDLEN 200000
//Set bits for Flags
#define PIPE 0x01   /* 0000 0001 */
#define STATEMENTS 0x02   /* 0000 0010 */
#define EXPAND 0x04   /* 0000 0100 */
#define WAIT 0x08   /* 0000 1000 */
#define REDIRECTION 0x10 /* 0001 0000 */


// Global variable//

int ArgcG;
char** ArgvG;
int ShiftArgc;
int ShiftOffset;
int status;
int ShiftCheck;
int SIGNAL;
int FLAGS = (PIPE | STATEMENTS | EXPAND | WAIT | REDIRECTION);
int pipehit;
int pipedone;
int RedirectionHit;
int whilestatement;
int ExpandHit;

/* Shell main */

int main (int mainargc, char **mainargv)
{
    char   buffer [LINELEN];
    int    len;
	char *mode = "r";
	int initialize = 1;
	//Store P1 environment if exists
	char* P1 = getenv("P1");

	//Initialize global argc and argv
	ArgcG = mainargc - 1; //not including script name
	ArgvG = mainargv; // arg[0] is shell name
	
	ShiftArgc = ArgcG;
	ShiftOffset = 0;
	
	//Handle SIGINT if occurs
	signal(SIGINT, signalHandler);
	
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
	
	pipehit = 0;
	RedirectionHit = 0;
	pipedone = 0;
	ExpandHit = 0;

	//Only display %% if Interactive run
	//No Script or File provided.
        /* prompt and get line */
	if (mainargc ==  1){		
		ShiftCheck = 1;
		if (P1 == NULL)
			fprintf (stderr, "%% ");
		else 
			fprintf (stderr, P1);
	}
	
	if (fgets (buffer, LINELEN, stdin) != buffer)
	  break;

        /* Get rid of \n at end of buffer. */
	len = strlen(buffer);
	if (buffer[len-1] == '\n')
	    buffer[len-1] = 0;

	/* Run it ... */
	processline (buffer, 0, 1, FLAGS);

    }

    if (!feof(stdin))
        perror ("read");
		
	if (feof(stdin)){
		fclose(stdin);
		return 0;
	}
	
    return 0;		/* Also known as exit (0); */
}


void processline(char *line, int inFD, int outFD, int FLAGS)
{
    pid_t  cpid;
	int argcount;
    char **argv;
	char pipeline[EXPANDLEN];
	char expandedline[EXPANDLEN];
	int coutFD = 2;
	int cinFD = 2;
	int cerrFD = 2;
	
	if (ExpandHit == 0) {
		if (expand(line, expandedline, EXPANDLEN)){
			return;
		}
	}
	
	
	//If piping was found in expand call PipeFunc
	if (pipehit == 1){
		pipehit = 0;
		PipeFunc(expandedline, pipeline, inFD, outFD, FLAGS);
	}

	
	//If redirection was found in expand call redirection
	if (RedirectionHit == 1){
		if ((pipehit == 1) || (pipedone == 1)){
			Redirection(pipeline, coutFD, cinFD, cerrFD);
			pipedone = 0;
		}
		else{
			Redirection(expandedline, coutFD, cinFD, cerrFD);
		}
	}
	
	if ((pipehit == 1) || (pipedone == 1)){
		argcount = arg_parse(pipeline, &argv);	
		pipedone = 0;
	}
	else{	
		argcount = arg_parse(expandedline, &argv);
	}
	
	//No arguments so no processing can be done
	if (argcount == 0){
		fprintf(stderr, "No arguments were given\n");
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
	  
		if (cinFD != 0) dup2(cinFD, 0);
		if (coutFD != 1) dup2(coutFD, 1);
		if (cerrFD != 1) dup2(cerrFD, 2);
		
		if ((dup2(outFD, 1)) < 0){
			perror("dup");
			return;
		}
		
		execvp (argv[0], argv);
		perror ("exec");
		exit (127);
    }

	
    /* Have the parent wait for child to complete */
    if (wait (&status) < 0)	
      perror ("wait");
	
    free(argv);
	
}

void signalHandler(int sig_num)
{
    //Just ignore SIGINT if not in while statement
	if (whilestatement == 1){
		kill(getpid(), SIGINT);
	}
	else{
		signal(SIGINT, signalHandler);
		SIGNAL = 1;
	}
	
}

void PipeFunc (char *line, char *pipeCommand, int inFD, int outFD, int Flags){
	int i = 0;
	int count = 0;
	//Keep track of current pipe index length
	//Hold index of last pipe that occurred for subsequent ones
	int slength = strlen(line);
	int fd[2];
	int LastFD;
	int firstpass = 0;
	
	while ((line[i] != '\0') && (i <= slength)){
		//Identifying Pipelines to process
		if (line[i] == '\0'){
			break;
		}
		if (line[i] == '|'){
			//Set last part of current command to 
			//end of line for this pipe
			pipeCommand[count] = '\0';	
			if(pipe(fd) < 0){
				perror("pipe");
			}
			if (firstpass == 0) {
				processline(pipeCommand, inFD, fd[1], FLAGS);
				//reset pipecommand and count for new command
				memset(pipeCommand, 0, slength);
				count = 0;
				//Close write end of pipe
				close(fd[1]);
				LastFD = fd[0];
				//read pipe to put into fdout
				firstpass = 1;
			}
			else{
				processline(pipeCommand, LastFD, fd[1], FLAGS);
				//reset pipecommand and count for new command
				memset(pipeCommand, 0, slength);
				count = 0;
				close(fd[1]);
				close(LastFD);
				LastFD = fd[0];
			}		
			i++;
		}
		else {
			pipeCommand[count] = line[i];
			i++;
			count++;
		}
	}
 
	//Make sure last part of command in null for end of line
	pipeCommand[count] = '\0';	
	FLAGS = (FLAGS &WAIT) & ~1;
    //processline(pipeCommand, LastFD, outFD, FLAGS);
    close(LastFD);
	pipehit = 0;
	pipedone = 1;
 
    return;
}
	

void Redirection (char *line, int coutFD, int cinFD, int cerrFD){
	int i = 0;
	//For Command iteration
	int count = 0;
	//For File iteration
	int Index = 0;
	int slength = strlen(line);
	int spaces;
	//Variables for max file length
	//and max command length
	char file[1024];
	char command[1024];
	char *input;
	int Append = 0;
	
	//remove quotes for processing files and commands
	//replace with space 
	for (i=0; i < slength; i++){
		if  (line[i] == '"')
			line[i] = ' ';
	}
	
	//count the number of leading spaces to ignore
	for (i=0; i < slength; i++){
		if (line[i] != ' '){
			break;
		}
		else if  (line[i] == ' ')
			spaces++;
	}
	i = spaces;
	
	while ((line[i] != '\0') && (i <= slength)) {
		//stderr to named File 2 must be first 
		//part of line, or follow a space.
		if (((line[i] == '2') && (line[i+1] == '>')) && 
		(((i-1 != 0) && (line[i-1] == ' ')) || (i == 0))){
			i++;
			Index = 0;
			if (line[i+1] == '>'){
				Append = 1;
			}
			while ((line[i] != ' ') && (i <= slength)){ 
				file[Index] = line[i];
				i++;
				Index++;
			}
			file[Index] = '\0';
			input = file;
			//Append stderr to end of file named
			if (Append == 1){
				if ((cerrFD = open(input, O_WRONLY|O_APPEND)) == -1){
					fprintf(stderr, "Cannot open given file for redirection\n");
				}
			}
			//Redirect stderr to file named, and create if doesn't exist
			else{
				if ((cerrFD = open(input, O_WRONLY|O_CREAT,0700)) == -1){
					fprintf(stderr, "Cannot open given file for redirection\n");
				}
			}
			if ((dup2(cerrFD, STDIN_FILENO)) == -1){
				fprintf(stderr, "Dup2 error\n");
			}			
			close(cerrFD);
		}
		//Redirection Processing for files
		if (line[i] == '>'){
			if (command[count-1] != ' '){
				command[count] = '\0';
			}
			else{
				command[count-1] = '\0';
			}
			count = 0;
			Index = 0;
			i++;
			if (line[i] == '>'){
				Append = 1;
				i++;
			}
			//Skip first space after > if there is one
			if (line[i] == ' '){
				i++;
			}
			while ((line[i] != ' ') && (i <= slength)){ 
				file[Index] = line[i];
				i++;
				Index++;
			}
			file[Index] = '\0';
			input = file;
			//Append stdout to end of file named
			if (Append == 1){
				if ((coutFD = open(input, O_WRONLY|O_APPEND)) == -1){
					fprintf(stderr, "Cannot open given file for redirection\n");
				}
			}
			//Redirect stdout to file named, and create if doesn't exist
			else{
				if ((coutFD = open(input, O_WRONLY|O_CREAT,0700)) == -1){
					fprintf(stderr, "Cannot open given file for redirection\n");
				}
			}	
			if ((dup2(coutFD, STDIN_FILENO)) == -1){
				fprintf(stderr, "Dup2 error\n");
			}
			close(coutFD);
		}
		//Attach existing file to standard input
		if (line[i] == '<'){
			i++;
			if (command[count-1] != ' '){
				command[count] = '\0';
			}
			else{
				command[count-1] = '\0';
			}
			count = 0;
			Index = 0;
			//Skip first space after < if there is one
			while (line[i] == ' '){
				i++;
			}
			while ((line[i] != ' ') && (i <= slength)){ 
				file[Index] = line[i];
				i++;
				Index++;
			}
			if (file[Index-1] == ' '){
				file[Index-1] = '\0';
			}
			else{
				file[Index] = '\0';
			}
			printf("file name:%s\n", file);
			input = file;
			cinFD = open(input, O_RDONLY);
			if ((dup2(cinFD, STDIN_FILENO))== -1){
				fprintf(stderr, "Dup2 error\n");
			}
			close(cinFD);		
		}
		//Saves commands to be used in redirection
		else{
			command[count] = line[i];
			count++;
		}
		i++;
	}
	RedirectionHit = 0;
	return;
}













	
	
	



