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

using namespace std;

#define MAX_PARAMS 1000
#define MAX_PARAM_SIZE 1024

char Param_name[MAX_PARAMS][MAX_PARAM_SIZE];
char Param_value[MAX_PARAMS][MAX_PARAM_SIZE];
int Params;
int Param_verbose = 1;

int read_parameters(char *paramfile) {
  FILE *fp;
  char name[MAX_PARAM_SIZE];
  Params = 0;

  fp = fopen("params.def", "r");
  if (fp != NULL) {
    while (fscanf(fp, "%s", name) == 1) {
      if (name[0] == '#') {
        int ch = 1;
        while (ch != '\n')
          ch = fgetc(fp);
        continue;
      } else {
        if (fscanf(fp, " = %[^\n]", Param_value[Params]) == 1) {

          //Remove end of line comments if they are there
          string temp_str(Param_value[Params]);
          size_t pos;
          string whitespaces(" \t\f\v\n\r");

          pos = temp_str.find("#");
          if (pos != string::npos)
            temp_str = temp_str.substr(0, pos);

          //trim trailing whitespace
          pos = temp_str.find_last_not_of(whitespaces);
          if (pos != string::npos) {
            if(pos != (temp_str.length() - 1))
              temp_str.erase(pos + 1);
          }
          else
            temp_str.clear(); //str is all whitespace

          strcpy(Param_value[Params], temp_str.c_str());

          strcpy(Param_name[Params], name);
          if (Param_verbose > 2) {
            printf("READ_PARAMS: %s = %s\n", Param_name[Params],
                Param_value[Params]);
          }
          Params++;
        } else {
          printf(
              "Help! Bad format in params.def file on line starting with %s\n",
              name);
          abort();
        }
      }
    }
  } else {
    printf("Help!  Can't read paramfile %s\n", "params.def");
    abort();
  }
  fclose(fp);

  fp = fopen(paramfile, "r");
  if (fp != NULL) {
    while (fscanf(fp, "%s", name) == 1) {
      if (name[0] == '#') {
        int ch = 1;
        while (ch != '\n')
          ch = fgetc(fp);
        continue;
      } else {
        if (fscanf(fp, " = %[^\n]", Param_value[Params]) == 1) {

          //Remove end of line comments if they are there
          string temp_str(Param_value[Params]);
          size_t pos;
          string whitespaces(" \t\f\v\n\r");

          pos = temp_str.find("#");
          if (pos != string::npos)
            temp_str = temp_str.substr(0, pos);

          //trim trailing whitespace
          pos = temp_str.find_last_not_of(whitespaces);
          if (pos != string::npos) {
            if(pos != (temp_str.length() - 1))
              temp_str.erase(pos + 1);
          }
          else
            temp_str.clear(); //str is all whitespace

          strcpy(Param_value[Params], temp_str.c_str());

          strcpy(Param_name[Params], name);
          if (Param_verbose > 2) {
            printf("READ_PARAMS: %s = %s\n", Param_name[Params],
                Param_value[Params]);
          }
          Params++;
        } else {
          printf("Help! Bad format in file %s on line starting with %s\n",
              paramfile, name);
          abort();
        }
      }
    }
  } else {
    printf("Help!  Can't read paramfile %s\n", paramfile);
    abort();
  }
  fclose(fp);

  if (Param_verbose > 1) {
    for (int i = 0; i < Params; i++) {
      printf("READ_PARAMS: %s = %s\n", Param_name[i], Param_value[i]);
    }
  }

  return Params;
}

int get_param(char *s, int *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%d", p)) {
        found = 1;
      }
    }
  }
  if (found) {
    if (Param_verbose) {
      printf("PARAMS: %s = %d\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Param_verbose) {
      printf("PARAMS: %s not found\n", s);
      fflush( stdout);
    }
    abort();
  }
  return 0;
}

int get_param(char *s, unsigned long *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%lu", p)) {
        found = 1;
      }
    }
  }
  if (found) {
    if (Param_verbose) {
      printf("PARAMS: %s = %lu\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Param_verbose) {
      printf("PARAMS: %s not found\n", s);
      fflush( stdout);
    }
    abort();
  }
  return 0;
}

int get_param(char *s, double *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%lf", p)) {
        found = 1;
      }
    }
  }
  if (found) {
    if (Param_verbose) {
      printf("PARAMS: %s = %f\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Param_verbose) {
      printf("PARAMS: %s not found\n", s);
      fflush( stdout);
    }
    abort();
  }
  return 0;
}

int get_param(char *s, float *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (sscanf(Param_value[i], "%f", p)) {
        found = 1;
      }
    }
  }
  if (found) {
    if (Param_verbose) {
      printf("PARAMS: %s = %f\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Param_verbose) {
      printf("PARAMS: %s not found\n", s);
      fflush( stdout);
    }
    abort();
  }
  return 0;
}

int get_param(char *s, char *p) {
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      if (strcpy(p, Param_value[i])) {
        found = 1;
      }
    }
  }
  if (found) {
    if (Param_verbose) {
      printf("PARAMS: %s = %s\n", s, p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Param_verbose) {
      printf("PARAMS: %s not found\n", s);
      fflush( stdout);
    }
    abort();
  }
  return 0;
}

int get_param_vector(char *s, vector < int > &p){
  char str[1024];
  int n;
  char *pch;
  int v;
  get_param(s,str);
  pch = strtok(str," ");
  if(sscanf(pch,"%d",&n) == 1){
    for (int i=0;i<n;i++){
      pch = strtok(NULL," ");
      if(pch == NULL) {
        printf("Help! bad param vector: %s\n", s);
        abort();
      }
      sscanf(pch,"%d",&v);
      p.push_back(v);
    }
  }
  else {
    printf("Incorrect format for vector %s\n",s);
    abort();
  }
  return n;
}

int get_param_vector(char *s, vector < double > &p){
  char str[1024];
  int n;
  char *pch;
  double v;
  get_param(s, str);
  pch = strtok(str," ");
  if (sscanf(pch, "%d", &n) == 1) {
    for (int i = 0; i < n; i++) {
      pch = strtok (NULL, " ");
      if (pch == NULL) {
        printf("Help! bad param vector: %s\n", s);
        abort();
      }
      sscanf(pch, "%lf", &v);
      p.push_back(v);
    }
  }
  else {
    printf("Incorrect format for vector %s\n",s);
    abort();
  }
  return n;
}

int get_param_vector(char *s, double *p) {
  char str[1024];
  int n;
  char *pch;
  get_param(s, str);
  pch = strtok(str, " ");
  if (sscanf(pch, "%d", &n) == 1) {
    for (int i = 0; i < n; i++) {
      pch = strtok(NULL, " ");
      if (pch == NULL) {
        printf("Help! bad param vector: %s\n", s);
        abort();
      }
      sscanf(pch, "%lf", &p[i]);
    }
  } else {
    abort();
  }
  return n;
}

int get_param_matrix(char *s, double ***p) {
  int n = 0;
  get_param((char *) s, &n);
  if (n) {
    double *tmp;
    tmp = new double[n];
    get_param_vector((char *) s, tmp);
    int temp_n = (int) sqrt((double) n);
    if (n != temp_n * temp_n) {
      printf("Improper matrix dimensions: matricies must be square,"
        " found dimension %i\n", n);
      abort();
    }
    n = temp_n;
    (*p) = new double *[n];
    for (int i = 0; i < n; i++)
      (*p)[i] = new double[n];
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        (*p)[i][j] = tmp[i * n + j];
      }
    }
    delete[] tmp;
    return n;
  }
  return -1;
}

bool does_param_exist(char *s) {

  bool found = false;
  for (int i = 0; i < Params && !found; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      found = true;
    }
  }

  return found;
}
