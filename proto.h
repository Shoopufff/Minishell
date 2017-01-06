/*
   Eric Ambrose
   February 14, 2014
   Assignment 4
   Proto.h
*/

/* Prototypes */

int builtin(int argc, char **argv);

int arg_parse(char *line, char ***argvp);

int expand (char *orig, char *new, int newsize);