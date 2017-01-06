/*    $Id: $    */
/*
   Eric Ambrose
   February 14, 2014
   Assignment 4
   Builtin.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>

#include "proto.h"

int i;
extern int ArgcG;
extern char **ArgvG;
//Hold original Value to be retained in Shift
//and Unshift so they may return to it and use
//for comparisons
extern int ShiftArgc;
extern int ShiftOffset;
extern int ShiftCheck;

void quit(int argc, char **argv) {
	int value;
	printf("Exiting the program....\n");
	if (argc ==1){
		exit(0);
	}
	else {
		value = atoi(argv[1]);
		exit(value);
	}	
}

void aecho(int argc, char **argv) {
	int i;
	int flag =0;
	
	for (i = 1; i < argc-1; i++){
		if (strcmp(argv[i], "-n") == 0) {
			flag = 1;
		}
		else{
			printf("%s%s", argv[i], " ");
		}
	}
	
	if (argc == 1){
		printf("\n");
	}
	
	if (flag == 1){
		printf("%s", argv[argc-1]);
	}
	else{
	printf("%s%s", argv[argc-1], "\n");
	}
	fflush(stdout);
}

void cd(int argc, char **argv){
        if (argv[1] == NULL) {
                chdir(getenv("HOME"));
        } 
		else {
            if (chdir(argv[1]) == -1) {
                fprintf(stderr, " %s: no such directory\n", argv[1]);
            }
        }
}

void envset(int argc, char **argv) {

	if (argc == 1){
		fprintf(stderr, "No environment variable given\n");
	}
	
	if (argc > 3){
		fprintf(stderr, "Too many arguments. \n");
	}

	if (argc != 3){
		fprintf(stderr, "Too few arguments. \n");
	}		
		
	if (setenv(argv[1], argv[2], 1) < 0) {
		perror("setenv");
	}
}

void envunset(int argc, char **argv) {

	if (argc == 1){
		fprintf(stderr, "No environment variable given\n");
	}
	
	if (argc > 2){
		fprintf(stderr, "Too many arguments. \n");
	}	
	
	if (unsetenv(argv[1]) < 0) {
        perror("unsetenv");
    }
}

void unshift(int argc, char **argv) {

	//unshift has no base for shift
	int shift;

	if (ShiftCheck == 1){
		fprintf(stderr, "Can't Unshift in interactive run\n");
	}
	else{
		//Unshift everything
		if (argc == 1){
			ArgcG = ShiftArgc;
			ShiftOffset = 0;
		}

		if (argc > 2){
			fprintf(stderr, "Too many arguments. \n");
		}	
	
		else{
			shift = atoi(argv[1]);
			if (shift <= ShiftOffset){
				ArgcG = ArgcG + shift;
				ShiftOffset = ShiftOffset - shift;
			}
			else {
				fprintf(stderr, "Can't Unshift, not enough arguments\n");
			}
		}
	}
}

void shift(int argc, char **argv) {

	//baseline for shift
	int shift = 1;
	
	if (ShiftCheck == 1){
		fprintf(stderr, "Can't Shift in interactive run\n");
	}
	else {
		//No N given so shift by 1
		if (argc == 1){	
			if (shift <= ArgcG){	
				ArgcG = ArgcG - 1;
				ShiftOffset = ShiftOffset + shift;
			}
			else {
				fprintf(stderr, "Can't Shift, not enough arguments\n");
			}
		}
	
		if (argc > 2){
			fprintf(stderr, "Too many arguments. \n");
		}	
		//N must have been given
		else{
			shift = atoi(argv[1]);
			if (shift <= ArgcG){
				ArgcG = ArgcG - shift;
				ShiftOffset = ShiftOffset + shift;
			}
			else {
				fprintf(stderr, "Can't Shift, not enough arguments\n");
			}
		}
	}
}

void sstat(int argc, char **argv){

	int i = 1;	

    if(argc < 2){    
        fprintf(stderr, "Too few arguments \n");
	}
	
	//Struct used to retrieve Stat values
    struct stat fileStat;
	
	time_t time = fileStat.st_mtime;
	char *timeString = ctime(&time);
	
	while(argv[i] != '\0'){
		if(stat(argv[i],&fileStat) < 0){    
			fprintf(stderr, "stat error\n");
		}
	
		printf("File Name = %s ",argv[i]);
		printf("Size = %zd bytes ",fileStat.st_size);
		printf("Links = %d ",fileStat.st_nlink);
	
		printf("User name = %s ", (getpwuid(fileStat.st_uid) -> pw_name));
		printf("Group name = %s ",(getgrgid(fileStat.st_gid) -> gr_name));
 
		printf("Permissions = ");
		printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
		printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
		printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
		printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
		printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
		printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
		printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
		printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");

		printf("Last Modified = %s ", timeString);
		
		//New line for next file
		printf("\n");
		
		i++;
		
	}	
}


int builtin(int argc, char **argv){

	//loop based on number of builtin functions
	if (strcmp(argv[0], "aecho") == 0) {
			aecho(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "exit") == 0) {
			quit(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "cd") == 0) {
			cd(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "envset") == 0) {
			envset(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "envunset") == 0) {
			envunset(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "shift") == 0) {
			shift(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "unshift") == 0) {
			unshift(argc, argv);
			return 1;
	}
	if (strcmp(argv[0], "sstat") == 0) {
			sstat(argc, argv);
			return 1;
	}
	
	return 0;
}
