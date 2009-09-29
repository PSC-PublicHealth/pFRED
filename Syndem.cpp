//
//
// File: SynDem.cpp
//

#include "Syndem.hpp"

// global runtime parameters
int Verbose;
int Test;
int Runs;
int Days;
unsigned long Seed;

// global file pointers
FILE *Statusfp;
FILE *Outfp;
FILE *Tracefp;

int main(int argc, char* argv[])
{
  char paramfile[80];
  if (argc > 1) {
    strcpy(paramfile, argv[1]);
  }
  else {
    strcpy(paramfile, "params");
  }
  printf("param file = %s\n", paramfile);

  // sprintf(filename, "status.txt");
  // Statusfp = fopen(filename, "w");
  Statusfp = stdout;

  setup(paramfile);
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
}

void setup(char *paramfile) {
  read_parameters(paramfile);
  get_global_parameters();
  get_population_parameters();
  get_disease_parameters();
  get_location_parameters();
  setup_diseases();
  setup_locations();
  setup_population();
  population_quality_control();
  location_quality_control();
}

void run_sim(int run) {
  char filename[80];
  unsigned long new_seed;

  sprintf(filename, "OUT/out%d.txt", run+1);
  Outfp = fopen(filename, "w");

  sprintf(filename, "OUT/trace%d.txt", run+1);
  Tracefp = fopen(filename, "w");

  fprintf(Statusfp, "\nStarting run %d\n", run);

  // allow us to replicate individual runs
  if (run > 0) { new_seed = Seed * 100 + run; }
  else { new_seed = Seed; }

  fprintf(Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);

  reset_locations(run);
  reset_population(run);
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


