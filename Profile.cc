/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Profile.cpp
//

#include "Profile.h"
#include <stdio.h>
#include <stdlib.h>
#include "Random.h"
#include "Params.h"
#include "Global.h"

Profile Prof[100];
int Profiles = 0;

void read_profiles(char *filename) {
  char name[80];

  if (Verbose) {
    fprintf(Statusfp, "read profiles entered\n"); fflush(Statusfp);
  }

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "Profile file %s not found\n", filename);
    exit(1);
  }

  int i;
  while (fscanf(fp, " Profile %d %s ", &i, name) == 2) {
    for (int d = 0; d < 7; d++) {
      double h, n, s, w, t;
      if (fscanf(fp,
		 " %*s Home %lf Nbrhood %lf School %lf Work %lf Travel %lf ",
		 &h, &n, &s, &w, &t) == 5) {
	
	Prof[i].set_prob_visit(d, 0, h);
	Prof[i].set_prob_visit(d, 1, n);
	Prof[i].set_prob_visit(d, 2, s);
	Prof[i].set_prob_visit(d, 4, w);
	Prof[i].set_prob_travel(d, t);
	Profiles++;
      }
      else {
	printf("Help! Bad format in profile %d\n", i);
	abort();
      }
    }
  }
}


int is_visited(int pos, int prof, int day) {
  return (RANDOM() < Prof[prof].get_prob_visit(day, pos));
}
