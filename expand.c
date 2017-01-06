/*    $Id: $    */
/*
   Eric Ambrose
   March 14, 2014
   Assignment 6
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
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>

#include "proto.h"

//Set bits for Flags as in main
#define PIPE 0x01   /* 0000 0001 */
#define STATEMENTS 0x02   /* 0000 0010 */
#define EXPAND 0x04   /* 0000 0100 */
#define WAIT 0x08   /* 0000 1000 */
#define REDIRECTION 0x10 /* 0001 0000 */

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
	int parencount = 0;
	int slength;
	int temp = 0;
	int firstpass = 0;
	int offset = 0;
	char *Env[1] = { 0 };
	char *ValEnv;
	int EnvI = 0;
	char pid[15] = { 0 };
	int pidI = 0;
	size_t counter = 0;
	int CommandLine = 0;
	int multiplier = 1;
	char StoreVal;
	int LastZero = 0;
	slength = strlen(line);
	char StatBuff[12];
	//Use for Comments truth values
	int commentquote = 0;
	int commented = 0;
	//Global Variables passed from Main
	extern int ArgcG;
	extern char **ArgvG;
	extern int ShiftOffset;
	extern int status;
	extern int FLAGS;
	extern int whilestatement;
	//extern int SIGNAL;
	
	//Use Password Struct to get directories for use in ~
	//Value to store user id if one is given or for current
	//shell user
	struct passwd *pw;
	uid_t uid;
	//Open current working directory in case
	//necessary for use later in expand. return 
	//with error if not possible
	DIR *Directory;
	if ((Directory = opendir(".")) == '\0'){
		fprintf(stderr, "couldn't open directory '.'\n");
		return 127;
	}
	//Current file looking at
	struct dirent *Current;
	char FileSuffix[256] = { 0 };
	char FileName[256] = { 0 };
	int FileProcessed = 0;
	//Size of Potential command
	char Command[1024];
	int fd[2];
	int ReadPipe;
	char PipeBuffer[1024];
	memset(PipeBuffer, 0, 1024);
	size_t len;
	
	extern int pipehit;
	extern int RedirectionHit;
	extern int ExpandHit;
	
	ExpandHit = 1;
	
	//Clear expandline for recurring runs
	for (i=0; i<EXPANDLEN; i++){
		expandedline[i] = '\0';
	}
 
	for ( i=0; i < slength; i++, temp++){
		checkagain:
		if (line[i] == '\0'){
			return expandedline[temp] = '\0';
		}
		//Once commented section is hit just ignore everything after it
		//And print to line as it would normally.
		if (commented == 1){
			expandedline[temp] = line[i];
			i++;
			temp++;
			goto checkagain;
		}
		//Just used to increment " to know whether comment is inside or not.
		if (line[i] == '"') {
			if (commentquote == 1){
				commentquote = 0;
			}
			else{
				commentquote = 1;
			}
		}
		//Don't expand and print regular output if not $ * ~ or # symbol
		if (line[i] != ('$' || '~' || '*' || '#')) {
			//Check for pipes or redirection for input
			//Ignore if inside comment or double quotes
			if (line[i] == '|'){
				if (commentquote != 1){
					pipehit = 1;
				}
			}
			if (line[i] == '>'){
				if (commentquote != 1){
					RedirectionHit = 1;
				}
			}
			if (line[i] == '<'){
				if (commentquote != 1){
					RedirectionHit = 1;
				}
			}
			expandedline[temp] = line[i];
		}
		//If commentted line ignore. Make sure it 
		//isn't in double quotes or adjacent to $
		if (line[i] == '#') {
			if (commentquote == 1){
				expandedline[temp] = line[i];
			}
			else {
				commented = 1;
			}
		}
		//Statement Processing Directly follows Possible Comments if 
		//Applicable, check for leading if Statement or While
		if ((commentquote != 1) && line[i] == ('i' || 'I')) {
			if ((i+1 <= slength) && (line[i+1] == ('f' || 'F'))){
				//if Statement found!
			}
		}		
		if ((commentquote != 1) && (line[i] == ('w' || 'W'))) {
			if ((i+1 <= slength) && (line[i+1] == ('h' || 'H'))){
				if ((i+1 <= slength) && (line[i+1] == ('i' || 'I'))){
					if ((i+1 <= slength) && (line[i+1] == ('l' || 'L'))){
						if ((i+1 <= slength) && (line[i+1] == ('e' || 'E'))){
							//While Statement Found!
							whilestatement = 1;
						}
					}
				}
			}
		}			
		//Processing for ~ Get Home Directory of Appropriate User
		//~ alone is User running shell, otherwise ~ followed 
		//by a username is the directory of the user named.
		//Check if ~ is directly after white space first
		if ((line[i] == '~') && (line[i-1] == ' ')){
			//to include first ~ increment offset to start
			offset = 1;
			counter = 0;
			firstpass = 1;
			expandedline[temp] = line[i];
			temp++;
			i++;
			//No user given so just use username of the user of shell
			//if (((i+1)<=slength) && (line[i+1] == (' ' || '\0'))){
			if ((line[i] == ' ') || (line[i] == '\0')) {
				uid = getuid();
				pw = getpwuid(uid);
				if (pw == '\0'){
					fprintf(stderr, "Could not get pw from user\n");
				}
				else {
					temp--;
					offset = strlen(pw->pw_dir);
					//Directory for user in pw_dir
					memcpy(FileName, pw->pw_dir, offset);
					while (FileName[counter] != '\0') {
						expandedline[temp] = FileName[counter];
						//Clear FileName simultaneously.
						FileName[counter] = '\0';
						temp++;
						counter++;
					}
					goto checkagain;
				}
			}
			//Not followed by empty space
			else{
				//expand until end bracket reached or end of line
				//keep track of offset to replace what made depending
				//on what value is in temp.
				while ((line[i] != ' ') && (i < slength)){
					if (firstpass == 1){
						expandedline[temp] = line[i];
						//point to beginning of input at this point
						//similar to how we use ${} later on.
						Env[0] = &expandedline[temp];
						firstpass = 0;
					}
					expandedline[temp] = line[i];
					temp++;
					i++;
					offset++;
				}
				//No UID exists with that name so leave the literal as is
				//go to just after finding the ~ and continue processing.
				if ((getpwnam(Env[0])) == '\0'){
					goto checkagain;
				}
				else {
					//account for original ~
					temp = temp - offset;
					pw = getpwnam(Env[0]);
					offset = strlen(pw->pw_dir);
					//go back to where temp initially started
					//Directory for user in pw_dir
					memcpy(FileName, (pw->pw_dir), offset);
					while(FileName[counter] != '\0'){
						expandedline[temp] = FileName[counter];
						//Clear FileName simultaneously.
						FileName[counter] = '\0';
						counter++;
						temp++;
					}
				}
			}
			offset = 0;
			goto checkagain;
		}		
		//Find * and expand for files
		if (line[i] == '*'){
			FileProcessed = 0;
			if ((i > 0) && (line[i-1] == '\\')){
				//literal * in expand
				expandedline[temp-1] = '*';
				i++;
				goto checkagain;
			}		
			i++;
			//All files except ones ending in '.'
			if (line[i] == ('\0' || ' ' || '"' || '*')){
				while ((Current = readdir(Directory)) != '\0'){
					memcpy(FileName, Current->d_name, strlen(Current->d_name));
					counter = 0;
					if (FileName[0] != '.'){
						while (FileName[counter] != '\0'){
							expandedline[temp] = FileName[counter];
							//Clear FileName simultaneously.
							FileName[counter] = '\0';
							counter++;
							temp++;
						}
						//Space between files
						expandedline[temp] = ' ';
						temp++;
					}
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
						//determine what file suffix search term
						//is and set FileSuffix string to equal it.
						FileSuffix[offset] = line[i];
						i++;
						offset++;
					}
				}
				//Null Terminate string
				FileSuffix[offset] = '\0';
				//Check against FileSuffix string and compare to end of FileName
				//and print those that match correct suffix
				while ((Current = readdir(Directory)) != '\0') {
					len = strlen(Current->d_name);
					if (len >= offset){
						memcpy(FileName, Current->d_name, strlen(Current->d_name));
						counter = 0;
						offset = len-offset;
						while (FileName[offset] != '\0'){
							if (FileSuffix[counter] != FileName[offset]){
								break;
							}
							//Clear Suffix Simultaneously
						    FileSuffix[offset] = '\0';
							offset++;
							counter++;
						}
						counter = 0;
						//copy file name to line
						while (FileName[counter] != '\0'){
							expandedline[temp] = FileName[counter];
							//Clear FileName simultaneously.
							FileName[counter] = '\0';
							temp++;
							counter++;
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
		if (line[i] == '$') {
			//Find parens () to find more commands to run in further shells
			if ((line[i] == '$') && (line[i+1] == '(')){
				counter = 0;
				offset = 0;
				i++;
				//Copy first paren to expandline
				//expandedline[temp] = line[i];
				//temp++;
				//Go past first paren to check next character
				i++;
				parencount = 1;
				//Process current arguments for expansion until
				//the matching paren is found for the initial one
				//so there may be other parens inside.
				while ((i <= slength) && (parencount != 0)){
					//Increment if another '(' found
					if (line[i] == '('){
						parencount++;
					}
					//Decrement if ')' is found
					if (line[i] == ')'){
						parencount--;
					}
					Command[counter] = line[i];
					//Keeps track of current size in between parens
					offset++;
					counter++;
					i++;
				}
				//Terminate end of current Command minus last paren
				Command[counter - 1] = '\0';
				if (parencount != 0){
					fprintf(stderr, "Uneven number of Parens");
				}
				else{
					//Now Process Command and read it into current
					//expand to get output here. 
					if(pipe(fd) < 0){
						perror("pipe");
					}
					processline(Command, fd[0], fd[1], FLAGS);
					//Close write end of pipe
					close(fd[1]);
					//read pipe to put into expandline
					if ((ReadPipe = read(fd[0], PipeBuffer, 1024)) >= 0){
						PipeBuffer[ReadPipe] = 0;
					}
					else{
						perror("read");
						close(fd[0]);
					}
					counter = 0;
					//Copy what was read into Expandline
					while (PipeBuffer[counter] != '\0'){
						//replace newlines with spaces in command
						if (PipeBuffer[counter] == '\n'){
							expandedline[temp] = ' ';
							temp++;
							counter++;
						}
						else{
							expandedline[temp] = PipeBuffer[counter];
							temp++;
							counter++;
						}
					}
					//If last part of command was space, delete it
					if (expandedline[temp-1] == '\n'){
						expandedline[temp-1] = ' ';
						temp--;
					}																		
				}
				goto checkagain;
			}					
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
					//Interactive run means can't have any argument but shell name
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
					//must be an interactive run so print only the 1 argument.
					expandedline[temp] = '1';
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
				offset = 0;
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
		}
	}
			
	closedir(Directory);

	
	return 0;
	
}