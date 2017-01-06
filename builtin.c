/*    $Id: $    */
/*
   Eric Ambrose
   January 22, 2014
   Assignment 2
   Builtin.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

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
		return;
	}
	
	if (flag == 1){
		printf("%s", argv[argc-1]);
	}
	else{
	printf("%s%s", argv[argc-1], "\n");
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
	
	return 0;
}
