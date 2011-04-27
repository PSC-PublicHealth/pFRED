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
#include <vector>
#include <map>

using namespace std;

int get_param(char *s, int *p);
int get_param(char *s, unsigned long *p);
int get_param(char *s, double *p);
int get_param(char *s, float *p);
int get_param(char *s, char *p);
int get_param(char *s, string &p);
int read_parameters(char *paramfile);
int get_param_vector(char *s, vector < int > &p);
int get_param_vector(char *s, vector < double > &p);
int get_param_vector(char *s, double *p);
int get_param_matrix(char *s, double ***p);
int get_param_map(char *s, map<string, double> *p);
int get_double_indexed_param_map(string s, int index_i, int index_j, map<string, double> *p);
int get_indexed_param_map(string s, int index, map<string, double> *p);
bool does_param_exist(char *s);
bool does_param_exist(string s);

template <typename T>
int get_param_from_string(string s, T *p) {
  char st[80];
  sprintf(st,"%s",s.c_str());
  int err = get_param(st,p);
  return err;
  }

template <typename T>
int get_indexed_param(string s, int index, T *p) {
  char st[80];
  sprintf(st, "%s[%d]",s.c_str(),index);
  int err = get_param(st,p);
  return err;
  }

template <typename T>
int get_double_indexed_param(string s, int index_i, int index_j, T* p) {
  char st[80];
  sprintf(st, "%s[%d][%d]",s.c_str(),index_i,index_j);
  int err = get_param(st,p);
  return err;
  }

template <typename T>
int get_indexed_param_vector(string s, int index, T* p) {
  char st[80];
  sprintf(st, "%s[%d]",s.c_str(),index);
  int err = get_param_vector(st,p);
  return err;
  }

template <typename T>
int get_double_indexed_param_vector(string s, int index_i, int index_j, T* p) {
  char st[80];
  sprintf(st, "%s[%d][%d]",s.c_str(),index_i,index_j);
  int err = get_param_vector(st,p);
  return err;
  }

#endif // _FRED_PARAMS_H
