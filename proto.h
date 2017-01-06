/*
   Eric Ambrose
   March 14, 2014
   Assignment 6
   Proto.h
*/

/* Prototypes */

int builtin(int argc, char **argv);

int arg_parse(char *line, char ***argvp);

int expand (char *orig, char *new, int newsize);

void processline(char *line, int inFD, int outFD, int FLAGS);

void signalHandler (int sig_num);

void PipeFunc (char *expandedline, char *pipeline, int inFD, int outFD, int FLAGS);

void Redirection (char *line, int coutFD, int cinFD, int cerrFD);