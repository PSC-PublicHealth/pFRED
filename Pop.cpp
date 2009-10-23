
/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Pop.cpp
//

#include "Pop.hpp"

extern int Verbose;
extern FILE *Statusfp;
extern FILE *Outfp;

char Popfile[80];
char Profilefile[80];
Person *Pop;
int Population;
int Index_cases;
set <int> *Exposed;
set <int> *Infectious;
int *S;
int *E;
int *I;
int *R;
double * Attack_rate;

void get_population_parameters() {
  get_param((char *) "popfile", Popfile);
  get_param((char *) "profiles", Profilefile);
  get_param((char *) "index_cases", &Index_cases);
}


void setup_population() {
  if (Verbose) {
    fprintf(Statusfp, "setup population entered\n");
    fflush(Statusfp);
  }

  int diseases = get_diseases();
  Exposed = new (nothrow) set <int> [diseases];
  if (Exposed == NULL) { printf("Help! Exposed allocation failure\n"); abort(); }

  Infectious = new (nothrow) set <int> [diseases];
  if (Infectious == NULL) { printf("Help! Infectious allocation failure\n"); abort(); }

  S = new (nothrow) int  [diseases];
  if (S == NULL) { printf("Help! S allocation failure\n"); abort(); }

  E = new (nothrow) int  [diseases];
  if (E == NULL) { printf("Help! E allocation failure\n"); abort(); }

  I = new (nothrow) int  [diseases];
  if (I == NULL) { printf("Help! I allocation failure\n"); abort(); }

  R = new (nothrow) int  [diseases];
  if (R == NULL) { printf("Help! R allocation failure\n"); abort(); }

  Attack_rate = new (nothrow) double  [diseases];
  if (Attack_rate == NULL) { printf("Help! Attack_rate allocation failure\n"); abort(); }

  // init population-disease lists
  for (int d = 0; d < diseases; d++) {
    Exposed[d].clear();
    Infectious[d].clear();
    Attack_rate[d] = 0.0;
  }

  read_profiles(Profilefile);
  read_population();

  if (Verbose) {
    fprintf(Statusfp, "setup population completed\n");
    fflush(Statusfp);
  }
}


void read_population() {

  if (Verbose) {
    fprintf(Statusfp, "read population entered\n"); fflush(Statusfp);
  }

  // read in population
  FILE *fp = fopen(Popfile, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "Popfile %s not found\n", Popfile);
    exit(1);
  }
  fscanf(fp, "Population = %d", &Population);
  if (Verbose) {
    fprintf(Statusfp, "Population = %d\n", Population);
    fflush(Statusfp);
  }

  // allocate population array
  Pop = new (nothrow) Person [Population];
  if (Pop == NULL) { printf ("Help! Pop allocation failure\n"); exit(1); }

  for (int p = 0; p < Population; p++) {
    int id, age, married, occ, prof, house, hood;
    int school, classroom, work, office, profile;
    char sex;

    // fprintf(Statusfp, "reading person %d\n", p); fflush(Statusfp);
    if (fscanf(fp, "%d %d %c %d %d %d %d %d %d %d %d %d %d",
	       &id, &age, &sex, &married, &occ, &prof, &house, &hood, &school,
	       &classroom, &work, &office, &profile) != 13) {
      fprintf(Statusfp, "Help! Read failure for person %d\n", p);
      abort();
    }
    Pop[p].setup(id, age, sex, married, occ, prof, house, hood, school,
		 classroom, work, office, profile);
    Pop[p].make_susceptible();
  }
  fclose(fp);
  if (Verbose) {
    fprintf(Statusfp, "finished reading population = %d\n", Population);
    fflush(Statusfp);
  }
}



void population_quality_control() {

  if (Verbose) {
    fprintf(Statusfp, "population quality control check\n");
    fflush(Statusfp);
  }

  // check population
  for (int p = 0; p < Population; p++) {
    if (Pop[p].get_places() == 0) {
      fprintf(Statusfp, "Help! Person %d has no home.\n",p);
      Pop[p].print(0);
      continue;
    }
    if (Pop[p].get_places() == 0) {
      fprintf(Statusfp,
	      "Help! Person %d has no place to go\n",p);
      Pop[p].print(0);
    }
  }

  if (Verbose) {
    int count[20];
    int total = 0;
    // age distribution
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < Population; p++) {
      int a = Pop[p].get_age();
      int n = a / 5;
      if (n < 20) { count[n]++; }
      else { count[19]++; }
      total++;
    }
    fprintf(Statusfp, "\nAge distribution: %d people\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     (c+1)*5, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    fprintf(Statusfp, "population quality control finished\n");
    fflush(Statusfp);
  }
}


void start_outbreak() {
  // create index cases
  for (int i = 0; i < Index_cases; i++) {
    int n = IRAND(0, Population-1);
    Pop[n].make_exposed(0, -1, -1, 'X', 0);
  }
}

void reset_population(int run) {

  if (Verbose) {
    fprintf(Statusfp, "reset population entered for run %d\n", run);
    fflush(Statusfp);
  }

  // init population-disease lists
  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {
    Exposed[d].clear();
    Infectious[d].clear();
    Attack_rate[d] = 0.0;
  }

  // add each person to the susceptible list for each place visited
  for (int p = 0; p < Population; p++){
    Pop[p].make_susceptible();
  }

  if (Verbose) {
    fprintf(Statusfp, "reset population completed\n");
    fflush(Statusfp);
  }
}

void update_exposed_population(int day) {
  int p;
  set<int>::iterator it;
  stack <int> TempList;

  if (Verbose > 1) {
    fprintf(Statusfp, "update_the_exposed for day %d\n", day);
    fflush(Statusfp);
  }

  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {

    for (it = Exposed[d].begin(); it != Exposed[d].end(); it++) {
      p = *it;
      if (Pop[p].get_infectious_date(d) == day) {
	TempList.push(p);
      }
    }

    if (Verbose > 1) {
      fprintf(Statusfp,
	      "update_the_exposed: new infectious for day %d disease %d = %d\n",
	      day, d, (int) TempList.size());
      fflush(Statusfp);
    }

    while (!TempList.empty()) {
      p = TempList.top();
      Pop[p].make_infectious(d);
      TempList.pop();
    }
  }

}

void update_infectious_population(int day) {
  int p;
  set<int>::iterator it;
  stack <int> TempList;

  if (Verbose > 1) {
    fprintf(Statusfp, "update_the_infectious for day %d\n", day);
    fflush(Statusfp);
  }

  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {

    for (it = Infectious[d].begin(); it != Infectious[d].end(); it++) {
      p = *it;
      // printf("inf = %d recov = %d\n", p, Pop[p].get_recovered_date(d));
      // fflush(stdout);
      if (Pop[p].get_recovered_date(d) == day) {
	TempList.push(p);
      }
    }

    // printf("Templist size = %d\n", (int) TempList.size()); fflush(stdout);
    while (!TempList.empty()) {
      p = TempList.top();
      // printf("top = %d\n", p ); fflush(stdout);
      Pop[p].make_recovered(d);
      TempList.pop();
    }
  }

  if (Verbose > 1) {
    fprintf(Statusfp, "update_the_infectious for day %d complete\n", day);
    fflush(Statusfp);
  }

}


void update_population_stats(int day) {

  if (Verbose > 1) {
    fprintf(Statusfp, "update pop stats\n"); 
    fflush(Statusfp);
  }

  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {
    S[d] = E[d] = I[d] = R[d] = 0;
    for (int p = 0; p < Population; p++) {
      char status = Pop[p].get_disease_status(d);
      S[d] += (status == 'S');
      E[d] += (status == 'E');
      I[d] += (status == 'I') || (status == 'i');
      R[d] += (status == 'R');
    }
    Attack_rate[d] = (100.0*(E[d]+I[d]+R[d]))/Population;
  }
}

void print_population_stats(int day) {

  if (Verbose > 1) {
    fprintf(Statusfp, "print pop stats for day %d\n", day);
    fflush(Statusfp);
  }

  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {
    int N = S[d]+E[d]+I[d]+R[d];
    fprintf(Outfp,
	    "Day %3d  Dis %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n",
	    day, d, S[d], E[d], I[d], R[d], N, Attack_rate[d]);
    fflush(Outfp);

    if (Verbose) {
      fprintf(Statusfp,
	      "Day %3d  Dis %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n\n",
	      day, d, S[d], E[d], I[d], R[d], N, Attack_rate[d]);
      fflush(Statusfp);
    }
  }
}

void print_population() {
  for (int p = 0; p < Population; p++) {
    Pop[p].print(0);
  }
}

void insert_into_exposed_list(int dis, int per) {
  Exposed[dis].insert(per);
}

void insert_into_infectious_list(int dis, int per) {
  Infectious[dis].insert(per);
}

void remove_from_exposed_list(int dis, int per) {
  Exposed[dis].erase(per);
}

void remove_from_infectious_list(int dis, int per) {
  if (Verbose > 2) {
    printf("remove from infectious list person %d\n", per);
    printf("current size of infectious list = %d\n", (int) Infectious[dis].size());
    fflush(stdout);
  }
  Infectious[dis].erase(per);
  if (Verbose > 2) {
    printf("final size of infectious list = %d\n", (int) Infectious[dis].size());
    fflush(stdout);
  }
}

int get_age(int per) {
  return Pop[per].get_age();
}

int get_role(int per, int dis) {
  return Pop[per].get_role(dis);
}

char get_disease_status(int per, int dis) {
  return Pop[per].get_disease_status(dis);
}

int is_place_on_schedule_for_person(int per, int day, int loc) {
  return Pop[per].is_on_schedule(day, loc);
}

double get_infectivity(int per, int dis) {
  return Pop[per].get_infectivity(dis);
}

double get_susceptibility(int per, int dis) {
  return Pop[per].get_susceptibility(dis);
}

void make_exposed(int per, int dis, int infector, int loc, char type, int day) {
  Pop[per].make_exposed(dis, infector, loc, type, day);
}

void add_infectee(int per, int dis) {
  Pop[per].add_infectee(dis);
}

void update_schedule(int per, int day) {
  Pop[per].update_schedule(day);
}

void get_schedule(int per, int *n, int *schedule) {
  Pop[per].get_schedule(n, schedule);
}

double get_attack_rate(int dis) {
  return Attack_rate[dis];
}
