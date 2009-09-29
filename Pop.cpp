//
//
// File: Pop.cpp
//

#include "Pop.hpp"

extern int Diseases;
extern int Verbose;
extern FILE *Statusfp;
extern FILE *Outfp;
extern vector<Place*> Loc;

char Popfile[80];
char Schedfile[80];
Person *Pop;
int Population;
int Index_cases;
set <int> *Exposed;
set <int> *Infectious;
int *S;
int *E;
int *I;
int *R;
int Start_day;
double * Attack_rate;

void get_population_parameters() {
  get_param((char *) "popfile", Popfile);
  get_param((char *) "schedfile", Schedfile);
  get_param((char *) "index_cases", &Index_cases);
}


void setup_population() {
  if (Verbose) {
    fprintf(Statusfp, "setup population entered\n");
    fflush(Statusfp);
  }

  Exposed = (set <int> *) malloc(Diseases*sizeof(set <int>));
  if (Exposed == NULL) { printf("Help! Exposed allocation failure\n"); abort(); }

  Infectious = (set <int> *) malloc(Diseases*sizeof(set <int>));
  if (Infectious == NULL) { printf("Help! Infectious allocation failure\n"); abort(); }

  S = (int *) malloc(Diseases*sizeof(int));
  if (S == NULL) { printf("Help! S allocation failure\n"); abort(); }

  E = (int *) malloc(Diseases*sizeof(int));
  if (E == NULL) { printf("Help! E allocation failure\n"); abort(); }

  I = (int *) malloc(Diseases*sizeof(int));
  if (I == NULL) { printf("Help! I allocation failure\n"); abort(); }

  R = (int *) malloc(Diseases*sizeof(int));
  if (R == NULL) { printf("Help! R allocation failure\n"); abort(); }

  Attack_rate = (double *) malloc(Diseases*sizeof(double));
  if (Attack_rate == NULL) { printf("Help! Attack_rate allocation failure\n"); abort(); }

  // init population-disease lists
  for (int d = 0; d < Diseases; d++) {
    Exposed[d].clear();
    Infectious[d].clear();
    Attack_rate[d] = 0.0;
  }

  read_population();
  read_schedules();
  set_occupations();

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
    int id, age;
    char sex;

    // fprintf(Statusfp, "reading person %d\n", p); fflush(Statusfp);
    if (fscanf(fp, "%d %d %c", &id, &age, &sex) != 3) {
      fprintf(Statusfp, "Help! Read failure for person %d\n", p);
      exit(1);
    }
    Pop[p].setup(id, age, sex);
  }
  fclose(fp);
  if (Verbose) {
    fprintf(Statusfp, "finished reading population = %d\n", Population);
    fflush(Statusfp);
  }
}


void read_schedules() {
  int id, day, loc;
  FILE *fp = fopen(Schedfile, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "Schedfile %s not found\n", Schedfile);
    exit(1);
  }
  if (Verbose) {
    fprintf(Statusfp, "reading schedules\n");
    fflush(Statusfp);
  }
  while (fscanf(fp, "%d %d %d", &id, &day, &loc) == 3) {
    if (id < Population)
      Pop[id].add_to_schedule(day, loc, Loc[loc]);
  }
  fclose(fp);
  if (Verbose) {
    fprintf(Statusfp, "finished reading schedules\n");
    fflush(Statusfp);
  }

  // add each person to the susceptible list for each place visited
  for (int p = 0; p < Population; p++){
    // Pop[p].print_schedule();
    Pop[p].make_susceptible();
  }
  if (Verbose) {
    fprintf(Statusfp, "finished building susceptible lists\n");
    fflush(Statusfp);
  }

}

void set_occupations() {
  if (Verbose) {
    fprintf(Statusfp, "setting occupations\n");
    fflush(Statusfp);
  }
  for (int p = 0; p < Population; p++){
    Pop[p].set_occupation();
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
    for (int day = 0; day < DAYS_PER_WEEK; day++) {
      if (Pop[p].get_schedule_size(day) == 0) {
	fprintf(Statusfp,
		"Help! Person %d has no place to go on day %d\n",p,day);
	Pop[p].print(0);
      }
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
  // start on a random day of the week
  Start_day = IRAND(0, 6);

  // create index cases
  for (int i = 0; i < Index_cases; i++) {
    int n = IRAND(0, Population-1);
    Pop[n].make_exposed(0, -1, -1, 0);
  }
}

void reset_population(int run) {

  if (Verbose) {
    fprintf(Statusfp, "reset population entered for run %d\n", run);
    fflush(Statusfp);
  }

  // init population-disease lists
  for (int d = 0; d < Diseases; d++) {
    Exposed[d].clear();
    Infectious[d].clear();
    Attack_rate[d] = 0.0;
  }

  // add each person to the susceptible list for each place visited
  for (int p = 0; p < Population; p++){
    // Pop[p].print_schedule();
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

  for (int d = 0; d < Diseases; d++) {

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

  for (int d = 0; d < Diseases; d++) {

    for (it = Infectious[d].begin(); it != Infectious[d].end(); it++) {
      p = *it;
      if (Pop[p].get_recovered_date(d) == day) {
	TempList.push(p);
      }
    }

    while (!TempList.empty()) {
      p = TempList.top();
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

  for (int d = 0; d < Diseases; d++) {
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

  // if (Verbose) {
  // fprintf(Statusfp, "print pop stats for day %d\n", day);
  // fflush(Statusfp);
  // }

  for (int d = 0; d < Diseases; d++) {
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
  Infectious[dis].erase(per);
}












