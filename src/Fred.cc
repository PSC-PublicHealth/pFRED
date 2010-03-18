/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Fred.cc
//

#include "Fred.h"
#include "Global.cc"
#include "Strain.h"
#include "Population.h"
#include "Locations.h"
#include "Params.h"
#include "Random.h"

char Paramfile[80];
int Random_start_day;

int main(int argc, char* argv[])
{
  time_t clock;		      // current date
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

  fprintf(Statusfp, "FRED started  ");
  time(&clock);
  fprintf(Statusfp, "%s", ctime(&clock));

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
  Random_start_day = (Start_day > 6);
  Pop.get_parameters();
  Loc.get_location_parameters();
  Loc.setup_locations();
  Pop.setup();
  Pop.population_quality_control();
  Loc.location_quality_control();
}

void run_sim(int run) {
  char filename[80];
  unsigned long new_seed;
  time_t clock;		      // current date

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
  fprintf(Statusfp, "FRED started  ");
  time(&clock);
  fprintf(Statusfp, "%s", ctime(&clock));

  // allow us to replicate individual runs
  if (run > 0 && Reseed_day == -1) { new_seed = Seed * 100 + run; }
  else { new_seed = Seed; }
  fprintf(Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);
  Loc.reset_locations(run);
  Pop.reset(run);
  if (Random_start_day) {
    // start on a random day of the week
    Start_day = IRAND(0, 6);
  }
  Pop.start_outbreak();
  for (int day = 0; day < Days; day++) {
    if (day == Reseed_day) {
      printf("************** reseed day = %d\n", day); fflush(stdout);
      INIT_RANDOM(new_seed + run);
    }
    printf("================\nsim day = %d\n", day); fflush(stdout);
    Pop.update(day);
    Pop.report(day);
    fprintf(Statusfp, "day %d finished  ", day);
    time(&clock);
    fprintf(Statusfp, "%s", ctime(&clock));
  }
  // Pop.print();
  fclose(Outfp);
  fclose(Tracefp);
}

void cleanup(int run) {
}

