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
#include "Global.h"
#include <math.h>
#include <string>
#include <sstream>

using namespace std;

#define MAX_PARAMS 1000
#define MAX_PARAM_SIZE 1024

char Param_name[MAX_PARAMS][MAX_PARAM_SIZE];
char Param_value[MAX_PARAMS][MAX_PARAM_SIZE];
int Params;

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
          if (Debug > 2) {
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
          if (Debug > 2) {
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
  
  if (Debug > 1) {
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
    if (Debug > 0) {
      printf("PARAMS: %s = %d\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Debug > 0) {
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
    if (Debug > 0) {
      printf("PARAMS: %s = %lu\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Debug > 0) {
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
    if (Debug > 0) {
      printf("PARAMS: %s = %f\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Debug > 0) {
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
    if (Debug > 0) {
      printf("PARAMS: %s = %f\n", s, *p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Debug > 0) {
      printf("PARAMS: %s not found\n", s);
      fflush( stdout);
    }
    abort();
  }
  return 0;
}

int get_param(char *s, string &p){
  int found = 0;
  for (int i = 0; i < Params; i++) {
    if (strcmp(Param_name[i], s) == 0) {
      stringstream ss;
      ss << Param_value[i];
      if(ss.str().size() > 0){
        p = ss.str();
        found = 1;
      }
    }
  }
  if (found) {
    if (Debug > 0) {
      printf("PARAMS: %s = %s\n", s, p.c_str());
      fflush( stdout);
    }
    return 1;
  } else {
    if (Debug > 0) {
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
    if (Debug > 0) {
      printf("PARAMS: %s = %s\n", s, p);
      fflush( stdout);
    }
    return 1;
  } else {
    if (Debug > 0) {
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

/*
  int get_param_map(char *s, map<char *, double> *p) {
  char str[1024];
  int n = 0;
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

  char *key, *valStr; double val;
  key = strtok(pch, ":");
  valStr = strtok(NULL, ":");

  if (valStr == NULL) {
  printf("Help! bad param vector: %s\n", s);
  abort();
  }
  sscanf(valStr, "%lf", &val);

  p->insert( pair<char *, double> (key, val) );
  }
  } else {
  abort();
  }
  return n;
  }
*/

int get_param_map(char *s, map<string, double> *p) {
  int err = 0;
  char str[1024];
  get_param(s, str);

  stringstream pairsStream(str);
  string kv_pair;
  getline(pairsStream, kv_pair, ' '); // read number of elements

  while(getline(pairsStream, kv_pair, ' ')) {
    stringstream ss(kv_pair);
    string key, val;

    if( getline(ss, key, ':') && getline(ss, val, ':') ) {
      double valDouble = atof(val.c_str());
      //p->insert( pair<string, double> ( (char *)key.c_str(), val1) );
      (*p)[key] = valDouble;
    }
    else {
      printf("Help! bad param vector: %s\n", s);
      abort();
    }
  }

  return err; // TODO: <<< is this correct?
}

int get_double_indexed_param_map(string s, int index_i, int index_j, map<string, double> *p) {
  char st[80];
  sprintf(st, "%s[%d][%d]",s.c_str(),index_i,index_j);
  int err = get_param_map(st,p);
  return err;
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

bool does_param_exist(string s) {
  char st[80];
  sprintf(st,"%s",s.c_str());
  return does_param_exist(st);
}
