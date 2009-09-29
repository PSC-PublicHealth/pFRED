//
//
// File Params.hpp
//
#ifndef _SYNDEM_PARAMS_H
#define _SYNDEM_PARAMS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#define MAX_PARAMS 1000
#define MAX_PARAM_SIZE 80

int get_param(char *s, int *p);
int get_param(char *s, unsigned long *p);
int get_param(char *s, double *p);
int get_param(char *s, float *p);
int get_param(char *s, char *p);
int read_parameters(char *paramfile);
int get_param_vector(char *s, double *p);

#endif // _SYNDEM_PARAMS_H
