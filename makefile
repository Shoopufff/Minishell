#    $Id: $
#Eric Ambrose
#Makefile
#Assignment 5
#February 28, 2014
#CSCI 352 W14

CC=gcc
CFLAGS= -g -Wall

Build: msh.o arg_parse.o builtin.o expand.o
	gcc -g -o msh msh.o arg_parse.o expand.o builtin.o

clean:
	rm -r msh msh.o arg_parse.o expand.o builtin.o

	
# dependency list

msh.o expand.o arg_parse.o builtin.o: proto.h

