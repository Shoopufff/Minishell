/*    $Id: $    */

/*
   Eric Ambrose
   February 13, 2014
   Assignment 4
   Globals.h
*/

#pragma once

/* C preprocessor defines for definition and initialization */

#ifndef EXTERN

#define EXTERN extern
#define INIT(x)

#else

#define EXTERN
#define INIT(x) = x 

#endif

/* global variables */

EXTERN char **ArgvG INIT(NULL);

EXTERN int ArgcG;