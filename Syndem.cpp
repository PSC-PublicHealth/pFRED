/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: SynDem.cpp
//

#include "SynDem.hpp"

// global runtime parameters
int Verbose;
int Test;
int Runs;
int Days;
unsigned long Seed;
int Start_day;
char Outfilebase[80];
char Tracefilebase[80];
char Paramfile[80];

// global file pointers
FILE *Statusfp;
FILE *Outfp;
FILE *Tracefp;

int main(int argc, char* argv[])
{
  if (argc > 1) {
    strcpy(Paramfile, argv[1]);
  }
  else {
    strcpy(Paramfile, "params");
  }
  printf("param file = %s\n", Paramfile);
  strcpy(Outfilebase, "OUT/out");
  strcpy(Tracefilebase, "OUT/trace");

  // sprintf(filename, "status.txt");
  // Statusfp = fopen(filename, "w");
  Statusfp = stdout;

  setup(Paramfile);
  for (int run = 0; run < Runs; run++) {
    run_sim(run);
  }
  // fclose(Statusfp);
  return 0;
}

void get_global_parameters() {
  get_param((char *) "verbose", &Verbose);
  get_param((char *) "test", &Test);
  get_param((char *) "runs", &Runs);
  get_param((char *) "days", &Days);
  get_param((char *) "seed", &Seed);
  get_param((char *) "outfile", Outfilebase);
  get_param((char *) "tracefile", Tracefilebase);
}

void setup(char *paramfile) {
  read_parameters(paramfile);
  get_global_parameters();
  get_population_parameters();
  get_disease_parameters();
  get_location_parameters();
  setup_diseases(Verbose);
  setup_locations();
  setup_population();
  population_quality_control();
  location_quality_control();
}

void run_sim(int run) {
  char filename[80];
  unsigned long new_seed;

  sprintf(filename, "%s%d.txt", Outfilebase, run+1);
  Outfp = fopen(filename, "w");
  if (Outfp == NULL) {
    printf("Help! Can't open %s\n", filename);
    abort();
  }

  sprintf(filename, "%s%d.txt", Tracefilebase, run+1);
  Tracefp = fopen(filename, "w");
  if (Tracefp == NULL) {
    printf("Help! Can't open %s\n", filename);
    abort();
  }


  fprintf(Statusfp, "\nStarting run %d\n", run);

  // allow us to replicate individual runs
  if (run > 0) { new_seed = Seed * 100 + run; }
  else { new_seed = Seed; }

  fprintf(Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);

  reset_locations(run);
  reset_population(run);
  // start on a random day of the week
  Start_day = IRAND(0, 6);
  start_outbreak();
  for (int day = 0; day < Days; day++) {
    printf("================\nsim day = %d\n", day); fflush(stdout);
    update_exposed_population(day);
    update_infectious_population(day);
    process_infectious_locations(day);
    update_population_stats(day);
    print_population_stats(day);
  }
  // print_population();
  fclose(Outfp);
  fclose(Tracefp);
}

void cleanup(int run) {
}


