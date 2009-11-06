/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Fred.cpp
//

#include "Fred.hpp"
#include "Global.cpp"
#include "Disease.hpp"
#include "Population.hpp"
#include "Locations.hpp"
#include "Params.hpp"
#include "Random.hpp"

char Paramfile[80];

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

void setup(char *paramfile) {
  read_parameters(paramfile);
  get_global_parameters();
  Pop.get_population_parameters();
  Disease::get_disease_parameters();
  Loc.get_location_parameters();
  Disease::setup_diseases(Verbose);
  Loc.setup_locations();
  Pop.setup_population();
  Pop.population_quality_control();
  Loc.location_quality_control();
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
  Loc.reset_locations(run);
  Pop.reset_population(run);
  // start on a random day of the week
  Start_day = IRAND(0, 6);
  Pop.start_outbreak();
  for (int day = 0; day < Days; day++) {
    printf("================\nsim day = %d\n", day); fflush(stdout);
    Pop.update_exposed_population(day);
    Pop.update_infectious_population(day);
    Pop.update_population_behaviors(day);
    Loc.process_infectious_locations(day);
    Pop.update_population_stats(day);
    Pop.print_population_stats(day);
  }
  // Pop.print_population();
  fclose(Outfp);
  fclose(Tracefp);
}

void cleanup(int run) {
}


