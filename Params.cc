/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File Params.cc
//

#include "Params.h"

#include <math.h>

#define MAX_PARAMS 1000
#define MAX_PARAM_SIZE 1024

char Param_name[MAX_PARAMS][MAX_PARAM_SIZE];
char Param_value[MAX_PARAMS][MAX_PARAM_SIZE];
int Params;
int Param_verbose = 1;

int get_param(char *s, int *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%d", p)) {
	if (Param_verbose == 1) { printf("PARAMS: %s = %d\n", s, *p); fflush(stdout); }
	found = 1;
      }      
    }
  }
  if (found) return 1;
  if (Param_verbose == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, unsigned long *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%lu", p)) {
	if (Param_verbose == 1) { printf("PARAMS: %s = %lu\n", s, *p); fflush(stdout); }
	found = 1;
      }      
    }
  }
  if (found) return 1;
  if (Param_verbose == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, double *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%lf", p)) {
	if (Param_verbose == 1) { printf("PARAMS: %s = %f\n", s, *p); fflush(stdout); }
	found = 1;
      }
    }
  }
  if (found) return 1;
  if (Param_verbose == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, float *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%f", p)) {
	if (Param_verbose == 1) { printf("PARAMS: %s = %f\n", s, *p); fflush(stdout); }
	found = 1;
      }
    }
  }
  if (found) return 1;
  if (Param_verbose == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, char *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (strcpy(p, Param_value[i])) {
	if (Param_verbose == 1) { printf("PARAMS: %s = %s\n", s, p); fflush(stdout); }
	found = 1;
      }
    }
  }
  if (found) return 1;
  if (Param_verbose == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int read_parameters(char *paramfile) {
  FILE *fp;
  Params = 0;

  fp = fopen("params.def", "r");
  if (fp != NULL) {
    // while (fscanf(fp, "%s = %s", Param_name[Params], Param_value[Params]) == 2) {
    while (fscanf(fp, "%s = %[^\n]", Param_name[Params], Param_value[Params]) == 2) {
      if (Param_verbose == 1) {
	printf("READ PARAMS: %s = %s\n", Param_name[Params], Param_value[Params]);
      }
      Params++;
    }
  }
  else {
    printf("Help!  Can't read paramfile %s\n", "params.def");
    abort();
  }
  fclose(fp);

  fp = fopen(paramfile, "r");
  if (fp != NULL) {
    // while (fscanf(fp, "%s = %s", Param_name[Params], Param_value[Params]) == 2) {
    while (fscanf(fp, "%s = %[^\n]", Param_name[Params], Param_value[Params]) == 2) {
      if (Param_verbose == 1) {
	printf("READ PARAMS: %s = %s\n", Param_name[Params], Param_value[Params]);
      }
      Params++;
    }
  }
  else {
    printf("Help!  Can't read paramfile %s\n", paramfile);
    abort();
  }
  fclose(fp);

  return Params;
}

int get_param_vector(char *s, double *p) {
  char str[1024];
  int n;
  char *pch;
  get_param(s, str);
  pch = strtok(str," ");
  if (sscanf(pch, "%d", &n) == 1) {
    for (int i = 0; i < n; i++) {
      pch = strtok (NULL, " ");
      if (pch == NULL) {
	printf("Help! bad param vector: %s\n", s);
	abort();
      }
      sscanf(pch, "%lf", &p[i]);
    }
  }
  else {
    abort();
  }
  return n;
}

int get_param_matrix(char *s, double ***p) {
  int n = 0;
  get_param((char *) s, &n);
  if (n) {
    double *tmp;
    tmp = new double [n];
    get_param_vector((char *)s, tmp);
    int temp_n = (int) sqrt((double) n);
    if (n != temp_n * temp_n) {
      printf("Improper matrix dimensions: matricies must be square,"
	     " found dimension %i\n", n);
      abort();
    }
    n = temp_n;
    (*p) = new double * [n];
    for (int i  = 0; i < n; i++) 
      (*p)[i] = new double [n];
    for (int i  = 0; i < n; i++) {
      for (int j  = 0; j < n; j++) {
	(*p)[i][j] = tmp[i*n+j];
      }
    }
    delete[] tmp;
    return n;
  }
  return -1;
}
