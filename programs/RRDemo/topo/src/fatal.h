/*
 * fatal.h
 *
 *  Created on: Jan 27, 2016
 *  Author: shu
 */

#include <stdio.h>
#include <stdlib.h>

#define Error(Str)      FatalError(Str)
#define FatalError(Str) fprintf(stderr, "%s\n", Str), exit(1)

