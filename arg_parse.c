/*    $Id: $    */
/*
   Eric Ambrose
   January 22, 2014
   Assignment 2
   Arg_Parse
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proto.h"

#define TEMPLEN 1024

int arg_parse(char *line, char ***argvp)
{
	char **Mpointer;
	char *dest;
	char *point;
	int argcounter = 0;
	int count = 0;
	int slength;
	int leadingspaces = 0;
	int i; //index value counter
	int ProcessingArg = 0; //Use as boolean for if arguments are being processed  
	char templine[TEMPLEN] = { 0 };
	point = templine;
	int temp = 0;
	slength = strlen(line);
	dest = line; //points to the line and is incremented to point to different
				 //sections as necessary to put into Mpointer array
	int firstpass;
	int quotecount = 0;
	
	//count the number of leading spaces as to ignore them when counting args
	for (i=0; i < slength; i++, dest++){
		if (line[i] != ' '){
			break;
		}
		else if  (line[i] == ' ')
			leadingspaces++;
	}

	//Must count arguments first in order to correctly Malloc the pointer array
	for ( i=leadingspaces; i < slength; i++){	
		if ( (line[i] == '"') && (ProcessingArg == 0) ){
			//Beginning quote found process until end quote.
			quotecount++;
			ProcessingArg = 1;
			firstpass = 1;
			i++; //check next character
			while ((line[i] != '"') && (i < slength)){ //continue until next quote is found
				//on first pass the argcounter is incremented as it is the
				//first no space character found unless it's quote
				//followed directly after by another quote
				if (firstpass == 1){
					argcounter++;
					firstpass = 0;
				}
				i++;
			}
			if (line[i] == '"'){
				quotecount++;
			}
		}
		//reading a line already and come across a quote, go until reach end quote
		else if ( (line[i] == '"') && (ProcessingArg == 1) ){
			i++;
			quotecount++;
			while ((line[i] != '"') && (i < slength)){
				i++;
			}
			if (line[i] == '"'){
				quotecount++;
			}
		}
		//this space symbolizes end of arg
	 	else if ( (line[i] == ' ') && (ProcessingArg == 1) ){ 
			ProcessingArg = 0;			
		}
		//Index of beginning of argument found not starting with a quote	
		else if ((line[i] != ' ') && (ProcessingArg == 0)){
			ProcessingArg = 1;		
			argcounter = argcounter + 1;
		}
	}
	
	//add extra spot for Null Pointer
	ProcessingArg = 0;
	*argvp = (char **)malloc(sizeof(char*)*(argcounter + 1)); 
	Mpointer = *argvp;
	int argloc[argcounter];
	
	printf("how many arguments?: %d\n", argcounter);
	
	
	if (quotecount % 2 != 0 ){
		fprintf(stderr, "There was uneven number of quotes\n");
		return 0;
	}
	
	//In case there are no arguments end here and just return right away
	if (argcounter == 0){
		return argcounter;
	}	
	else{
	
	//Get the beginning of each argument to put into the array that we malloc'd
	//Use src and dest to remove the quotes by only storing what we want to store
	//into dest and keeping everything else to read properly in src
		for ( i=leadingspaces; i < slength; i++, dest++){
			if ( (line[i] == '"') && (line[i+1] == '"') ){
					//skip both quotes because they are meaningless
					i++;
					dest++;
			}
			else if ( (line[i] == '"') && (ProcessingArg == 0) ){
				//Beginning quote found, process until end quote.
				ProcessingArg = 1;
				firstpass = 1;
				i++; //check next character
				dest++;
				//continue until next quote is found
				while ((line[i] != '"') && (i < slength)){
					if (firstpass == 1){
					    templine[temp] = line[i];
						Mpointer[count] = point;
						argloc[count] = temp;
						count++;
						firstpass = 0;
					}
					templine[temp] = line[i];
					dest++;
					i++;
					temp++;
					point++;
				}
			}
			else if ( (line[i] == '"') && (ProcessingArg == 1) ){
				//Beginning quote in the middle of current argument processing
				i++;
				dest++;
				while ((line[i] != '"') && (i < slength)){
					templine[temp] = line[i];
					temp++;
					point++;
					i++;
					dest++;
				}
			}
			//Index of beginning of argument
			else if ( (line[i] != ' ') && (ProcessingArg == 0) ){  
				ProcessingArg = 1;
				templine[temp] = line[i];
				Mpointer[count] = point;  
				//Set array at [count] to point to start of argument in line	
				argloc[count] = temp;
				count++;
				temp++;
				point++;
			}
			//this space symbolizes end of arg
			else if ( (line[i] == ' ') && (ProcessingArg == 1) ){ 
				//check if previous character was a quote
				//change to remove quote if so
				if (line[i-1] == '"'){
					ProcessingArg = 0;
					templine[temp] = '\0';
				}
				else{
					ProcessingArg = 0;
					line[i] = '\0';
					templine[temp] = line[i];
				}
				temp++;
				point++;
			}
			else{
				templine[temp] = line[i];
				temp++;
				point++;
			}
		}
	}
	
	temp++;
	templine[temp] = '\0';
	
	for (i=0; i<slength; i++){
	line[i] = templine[i];
	}
	
	count = 0;
	
	for (i=0; i<argcounter; i++){
		Mpointer[i] = &line[argloc[count]];
		count++;
	}
	
	
	Mpointer[count] = '\0'; //Adding null pointer to end of the array
	
	return argcounter;

}