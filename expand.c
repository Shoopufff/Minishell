/*    $Id: $    */
/*
   Eric Ambrose
   February 14, 2014
   Assignment 4
   Expand.c
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#include "proto.h"

int EXPANDLEN = 2000;

//Use in $n to reverse number read as it
//is read backwards during processing.
int reverseint(int number){
    int inverse = 0;
	
    while (number>0){
        inverse = inverse * 10 + (number%10);
        number = number / 10;
    }
    
	return inverse;
}


int expand(char *line, char *expandedline, int EXPANDLEN){

	int i = 0;
	int quotecount = 0;
	int slength;
	int temp = 0;
	int firstpass = 0;
	int offset = 0;
	char *Env[1] = { 0 };
	char *ValEnv;
	int EnvI = 0;
	char pid[15] = { 0 };
	int pidI = 0;
	int counter = 0;
	int CommandLine = 0;
	int multiplier = 1;
	char StoreVal;
	int LastZero = 0;
	slength = strlen(line);
	char StatBuff[12];
	extern int ArgcG;
	extern char **ArgvG;
	extern int ShiftOffset;
	extern int status;
	//Open current working directory in case
	//necessary for use later in expand.
	DIR *Directory = opendir(".");
	struct dirent *Current;
	char FileSuffix[100];
	char FileName[100];
	int FileProcessed = 0;
	size_t len;
	
	
	//Clear expandline for recurring runs
	for (i=0; i<EXPANDLEN; i++){
		expandedline[i] = '\0';
	}

	for ( i=0; i < slength; i++, temp++){
		checkagain:
		if (line[i] == '\0'){
			return expandedline[temp] = '\0';
		}
		if (line[i] == '*'){
			FileProcessed = 0;
			if ((i > 0) && (line[i-1] == '\\')){
				//literal * in expand
				expandedline[temp-1] = '*';
				i++;
				goto checkagain;
			}		
			i++;
			printf("I made it to all files, not really\n");
			//All files except ones ending in '.'
			if (line[i] == ('\0' || ' ' || '"' || '*')){
				printf("I made it to all files1\n");
				if ((Current->d_name[0] != '.')){
					memcpy(FileName, Current->d_name, strlen(Current->d_name));
					counter = 0;
					printf("I made it to all files2\n");
					while (FileName[counter] != '\0'){
						expandedline[temp] = FileName[counter];
						temp++;
						printf("I made it to all files\n");
					}
					//Space between files
					expandedline[temp] = ' ';
					temp++;
				}
				FileProcessed = 1;
			}
			//All files that have a specific Suffix attached
			//Stop processing and error out if '/' encountered
			else{
				counter = 0;
				offset = 0;
				while (line[i] != (' ' || '\0' || '"' || '*')){
					if (line[i] == '/'){
						fprintf(stderr, "Error with '*' usage no '/' allowed\n");
					}
					else {
						FileSuffix[offset] = line[i];
						i++;
						offset++;
					}
				}
				//Null Terminate string
				FileSuffix[offset] = '\0';
				while ((Current = readdir(Directory))) {
					len = strlen(Current->d_name);
					if ((Current->d_name[0] != '.') && ((len >= offset) && 
					(strcmp(Current->d_name + len - offset, FileSuffix) == 0))) {
						memcpy(FileName, Current->d_name, strlen(Current->d_name));
						counter = 0;
						while (FileName[counter] != '\0'){
							expandedline[temp] = FileName[counter];
							temp++;
							printf("I made it to suffixrules");
						}
						//Space between files
						expandedline[temp] = ' ';
						temp++;
					}
					FileProcessed = 1;
				}
			}
			if ((expandedline[temp-1] == ' ') && (FileProcessed == 1)){
				//remove trailing space after files processed.
				temp--;
			}
			i++;
			goto checkagain;
		}
		if (line[i] != '$') {
			expandedline[temp] = line[i];
		}
		if (line[i] == '$') {
			if ((line[i] == '$') && (isdigit((int)line[i+1]))){
				counter = 0;
				//Check to see what the number is
				i++;
				multiplier = 1;
				StoreVal = line[i] - '0';
				CommandLine = StoreVal;
				while (isdigit((int)line[i+1])) {
					i++;
					multiplier = multiplier * 10;
					StoreVal = line[i] - '0';
					CommandLine = CommandLine + (StoreVal * multiplier);
					if (StoreVal == 0){
						LastZero = 1;
					}
					else{
						LastZero = 0;
					}
				}
				//Reverse Number to read correctly.
				//If last digit is Zero it won't reverse
				//Properly so increment before reverse
				//Then decrement to get proper number
				if (LastZero == 1){
					CommandLine = CommandLine * multiplier;
					CommandLine++;
					CommandLine = reverseint(CommandLine);
					CommandLine--;
				}
				else{
					CommandLine = reverseint(CommandLine);
				}	
				LastZero = 0;
				//Interactive run if ArgcG only has 1 argument
				if (ArgcG == 1)
					//Give Shell name if $0 given
					if (CommandLine == 0){
						while (ArgvG[0][counter] != '\0'){
							expandedline[temp] = ArgvG[0][counter];
							temp++;
							counter++;
						}
					}
					//N != 0 so print nothing
					else{
						expandedline[temp] = ' ';
						temp++;
					}	
				else {
					//Ignore Shell name so increment by 1
					CommandLine = CommandLine + 1;
					if (CommandLine > ArgcG) {
						expandedline[temp] = ' ';
						temp++;
					}
					else {
						while (ArgvG[CommandLine][counter] != '\0'){
							//Use ShiftOffset in case Shift has been used to modify
							//Where to point in MainArgv and react accordingly
							expandedline[temp] = ArgvG[(int)CommandLine+ShiftOffset][counter];
							temp++;
							counter++;
						}
					}
				}
				i++;
				if (line[i] == '$'){
					//makes sure a $ symbol isn't missed by looking
					//at next element to process it again if it's another $
					goto checkagain;
				}
				else{
					expandedline[temp] = line[i];
				}
			}
			if ((line[i] == '$') && (line[i+1] == '#')){
				i++;
				i++;
				if (ArgcG > 1){
					expandedline[temp] = ArgcG+'0';
					temp++;
				}
				else{
					expandedline[temp] = ' ';
					temp++;
				}
				if (line[i] == '$'){
					//makes sure a $ symbol isn't missed by looking
					//at next element to process it again if it's another $
					goto checkagain;
				}
				else{
					expandedline[temp] = line[i];
				}
			}	
			if ((line[i] == '$') && (line[i+1] == '?')){
				i++;
				i++;
				if (WIFEXITED(status)) {
					counter = 0;
					printf("status %d\n", WEXITSTATUS(status));
					snprintf(StatBuff, 12, "%d", WEXITSTATUS(status)); 
					while (StatBuff[counter] != '\0'){
						expandedline[temp] = StatBuff[counter];
						temp++;
						counter++;
					}
				}
				if (line[i] == '$'){
					//makes sure a $ symbol isn't missed by looking
					//at next element to process it again if it's another $
					goto checkagain;
				}
				else{
					expandedline[temp] = line[i];
				}
			}
			if ((line[i] == '$') && (line[i+1] == '$')){
				//Get PID number if $ follows directly after $
				i++;
				i++;
				sprintf(pid, "%ld", (long)getpid());
				while (pid[pidI] != '\0'){
					expandedline[temp] = pid[pidI];
					temp++;
					pidI++;
				}
				pidI = 0;
				if (line[i] == '$'){
					//makes sure a $ symbol isn't missed by looking
					//at next element to process it again if it's another $
					goto checkagain;
				}
				else{
					expandedline[temp] = line[i];
				}
			}
			if ((line[i] == '$') && (line[i+1] == '{')){
				//Begin expansion because bracket is in next spot
				i++;
				quotecount++;
				firstpass = 1;
				i++; //check next character
				expandedline[temp] = ' ';
				temp++;
				//expand until end bracket reached or end of line
				while ((line[i] != '}') && (i < slength)){ 
					if (firstpass == 1){
					    expandedline[temp] = line[i];
						Env[0] = &expandedline[temp]; //point to first part of environment variable
						firstpass = 0;
					}
					expandedline[temp] = line[i];
					i++;
					temp++;
					offset++;
				}
				//reached end of line or end bracket
				if ((line[i] == '}') && (firstpass == 0)){
					offset++;
					quotecount++;
					i++;
				}
				if ((line[i] == '}') && (firstpass == 1)){
					//there was nothing inside quotes as firstpass wasn't changed.
					//return null argument as there was nothing
					fprintf(stderr, "Bad Substitution, nothing inside braces\n");
					return -1;
				}	
				//if no end bracket then report error
				if (quotecount % 2 != 0 ){
					fprintf(stderr, "There was uneven number of braces\n");
					return -1;
				}
				ValEnv = getenv(Env[0]);
				temp = temp - offset;
				offset = 0;
				if (ValEnv != NULL){
					while(ValEnv[EnvI] != '\0'){
						expandedline[temp] = ValEnv[EnvI];
						EnvI++;
						temp++;
					}
					EnvI = 0;
				}
				else{
					expandedline[temp] = ' ';
				}			
				quotecount = 0;
				if (line[i] == '$'){
					goto checkagain;
				}
				else{
					expandedline[temp] = line[i];
				}
			}
			if (line[i] == '$'){
				//if nothing important is next just continue as usual
				expandedline[temp] = line[i];
			}
		}
	}
			
	
	//for (i=0; i < EXPANDLEN; i++){
	//	printf("%c", expandedline[i]);
	//}
	//printf("\n");
	
	return 0;
	
}