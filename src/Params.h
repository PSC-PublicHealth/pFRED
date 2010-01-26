/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File Params.h
//
#ifndef _FRED_PARAMS_H
#define _FRED_PARAMS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

int get_param(char *s, int *p);
int get_param(char *s, unsigned long *p);
int get_param(char *s, double *p);
int get_param(char *s, float *p);
int get_param(char *s, char *p);
int read_parameters(char *paramfile);
int get_param_vector(char *s, double *p);
int get_param_matrix(char *s, double ***p);
bool does_param_exist(char *s);

#endif // _FRED_PARAMS_H
