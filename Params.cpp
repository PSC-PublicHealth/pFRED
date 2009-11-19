/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File Params.cpp
//

#include "Params.hpp"

#define MAX_PARAMS 1000
#define MAX_PARAM_SIZE 1024

char Param_name[MAX_PARAMS][MAX_PARAM_SIZE];
char Param_value[MAX_PARAMS][MAX_PARAM_SIZE];
int Params;
int Param_debug = 1;

int get_param(char *s, int *p) {
  int i;
  // printf("BEFORE PARAMS: %s = %d\n", s, *p); fflush(stdout);
  for (i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      // printf("match found %s = %s\n", Param_name[i], Param_value[i]); fflush(stdout);
      sscanf(Param_value[i], "%d", p);
      if (Param_debug == 1) { printf("PARAMS: %s = %d\n", s, *p); fflush(stdout); }
      return 1;
    }
  }
  if (Param_debug == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, unsigned long *p) {
  int i;
  // printf("BEFORE PARAMS: %s = %lu\n", s, *p); fflush(stdout);
  for (i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      // printf("match found %s = %s\n", Param_name[i], Param_value[i]); fflush(stdout);
      sscanf(Param_value[i], "%lu", p);
      if (Param_debug == 1) { printf("PARAMS: %s = %lu\n", s, *p); fflush(stdout); }
      return 1;
    }
  }
  if (Param_debug == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, double *p) {
  int i;
  // printf("BEFORE PARAMS: %s = %f\n", s, *p); fflush(stdout);
  for (i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      // printf("match found %s = %s\n", Param_name[i], Param_value[i]); fflush(stdout);
      sscanf(Param_value[i], "%lf", p);
      if (Param_debug == 1) { printf("PARAMS: %s = %f\n", s, *p); fflush(stdout); }
      return 1;
    }
  }
  if (Param_debug == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, float *p) {
  int i;
  // printf("BEFORE PARAMS: %s = %f\n", s, *p); fflush(stdout);
  for (i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      // printf("match found %s = %s\n", Param_name[i], Param_value[i]); fflush(stdout);
      sscanf(Param_value[i], "%f", p);
      if (Param_debug == 1) { printf("PARAMS: %s = %f\n", s, *p); fflush(stdout); }
      return 1;
    }
  }
  if (Param_debug == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int get_param(char *s, char *p) {
  int i;
  // printf("BEFORE PARAMS: %s = %s\n", s, *p); fflush(stdout);
  for (i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      // printf("match found %s = %s\n", Param_name[i], Param_value[i]); fflush(stdout);
      strcpy(p, Param_value[i]);
      if (Param_debug == 1) { printf("PARAMS: %s = %s\n", s, p); fflush(stdout); }
      return 1;
    }
  }
  if (Param_debug == 1) { printf("PARAMS: %s not found\n", s); fflush(stdout); }
  abort();
  return 0;
}

int read_parameters(char *paramfile) {
  FILE *fp;
  Params = 0;
  fp = fopen(paramfile, "r");
  if (fp != NULL) {
    // while (fscanf(fp, "%s = %s", Param_name[Params], Param_value[Params]) == 2) {
    while (fscanf(fp, "%s = %[^\n]", Param_name[Params], Param_value[Params]) == 2) {
      if (Param_debug == 1) {
	printf("READ PARAMS: %s = %s\n", Param_name[Params], Param_value[Params]);
      }
      Params++;
    }
  }
  else {
    printf("Help!  Can't read paramfile %s\n", paramfile);
    abort();
  }
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
      // printf("p[%d] = %f\n",i,p[i]); fflush(stdout);
    }
  }
  else {
    abort();
  }
  return n;
}

