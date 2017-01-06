#    $Id: $
#Eric Ambrose
#Makefile
#Assignment 3
#CSCI 352 W14

CC=gcc
CFLAGS= -g -Wall

Build: msh.o arg_parse.o builtin.o
	gcc -g -o msh msh.o arg_parse.o builtin.o

clean:
	rm -r msh msh.o arg_parse.o builtin.o


# dependency list

msh.o arg_parse.o builtin.o: proto.h

