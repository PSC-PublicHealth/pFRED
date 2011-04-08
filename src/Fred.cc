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
#include "Global.h"
#include "Disease.h"
#include "Population.h"
#include "Place_List.h"
#include "Grid.h"
#include "Params.h"
#include "Random.h"
#include "Vaccine_Manager.h"
#include "Date.h"

Date * Sim_Date;

int main(int argc, char* argv[]) {
  time_t clock;					// current date
  int run;					// number of current run
  unsigned long new_seed;
  char filename[256];
  char directory[256];
  char paramfile[256];

  if (argc > 1) { strcpy(paramfile, argv[1]); }
  else { strcpy(paramfile, "params"); }
	
  if (argc > 2) { sscanf(argv[2], "%d", &run); }
  else { run = 1; }

  if (argc > 3) { strcpy(directory, argv[3]); }
  else { strcpy(directory, ""); }

  Statusfp = stdout;
  time(&clock);
  fprintf(Statusfp, "FRED started %s", ctime(&clock));
  fprintf(Statusfp, "param file = %s\n", paramfile);
  fflush(Statusfp);

  // get runtime parameters
  read_parameters(paramfile);
  get_global_parameters();
  Pop.get_parameters();

  if (strcmp(directory, "") == 0) {
    // use the directory in the params file
    strcpy(directory, Output_directory);
    // printf("directory = %s\n",directory);
  }

  // create the output directory, if necessary
  mode_t mask;        // the user's current umask
  mode_t mode = 0777; // as a start
  mask = umask(0); // get the current mask, which reads and sets...
  umask(mask);     // so now we have to put it back
  mode ^= mask;    // apply the user's existing umask
  if (0!=mkdir(directory, mode) && EEXIST!=errno) // make it
    err(errno, "mkdir(Output_directory) failed");      // or die

  // open output files
  sprintf(filename, "%s/out%d.txt", directory, run);
  Outfp = fopen(filename, "w");
  if (Outfp == NULL) {
    printf("Help! Can't open %s\n", filename); abort();
  }
  Tracefp = NULL;
  if (strcmp(Tracefilebase, "none") != 0) {
    sprintf(filename, "%s/trace%d.txt", directory, run);
    Tracefp = fopen(filename, "w");
    if (Tracefp == NULL) {
      printf("Help! Can't open %s\n", filename); abort();
    }
  }
  VaccineTracefp = NULL;
  if (strcmp(VaccineTracefilebase, "none") != 0) {
    sprintf(filename, "%s/vacctr%d.txt", directory, run);
    VaccineTracefp = fopen(filename, "w");
    if (VaccineTracefp == NULL) {
      printf("Help! Can't open %s\n", filename); abort();
    }
  }
	
  // Date Setup
  INIT_RANDOM(Seed);
  Random_start_day = (Epidemic_offset > 6);

  // If Start_date parameter is "today", then do default constructor
  if (strncmp(Start_date, "today", 5)==0)
    Sim_Date = new Date();
  else
    Sim_Date = new Date(string(Start_date), Date::MMDDYYYY);
  Sim_Date->setup(directory, Days);

  // set random number seed based on run number
  if (run > 1 && Reseed_day == -1) { new_seed = Seed * 100 + (run-1); }
  else { new_seed = Seed; }
  fprintf(Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);

  time(&clock);
  fprintf(Statusfp, "\nFRED run %d started %s\n", run, ctime(&clock));
  fflush(Statusfp);
	
  // initializations

  // set up geographical patches
  Environment.setup();

  // read in the household, schools and workplaces
  Places.read_places();

  // read in the population and have each person enroll
  // in each favorite place identified in the population file
  Pop.setup();

  // define FRED-specific places
  Places.setup_classrooms();
  Places.setup_offices();

  // define FRED-specific social networks,
  // and have each person enroll as needed
  Pop.assign_classrooms();
  Pop.assign_offices();

  // after all enrollments, prepare to receive visitors
  Places.prepare();

  // record the favorite places for households within each patch
  Environment.record_favorite_places();

  if (Quality_control) {
    Pop.quality_control();
    Places.quality_control();
    Environment.quality_control();
  }

  // allow for an offset in the start of the epidemic
  if (Random_start_day) {
    // cycle through days of the week for start day
    Epidemic_offset = (run-1) % 7;
  }

  for (int day = 0; day < Days; day++) {
    if (day == Reseed_day) {
      printf("************** reseed day = %d\n", day); fflush(stdout);
      INIT_RANDOM(new_seed + run - 1);
    }
    printf("================\nsim day = %d\n", day); fflush(stdout);
    Places.update(day);
    Pop.begin_day(day);
    Pop.get_visitors_to_infectious_places(day);
    Pop.transmit_infection(day);
    Pop.end_day(day);
    Pop.report(day);
    fprintf(Statusfp, "day %d finished  ", day);
    // incremental trace
    if (Incremental_Trace && day && !(day%Incremental_Trace))
      Pop.print(1, day);
    time(&clock);
    fprintf(Statusfp, "%s", ctime(&clock));
  }

  // finish up
  Pop.end_of_run();
  // fclose(Statusfp);
  fclose(Outfp);
  if (Tracefp != NULL) fclose(Tracefp);
  if (VaccineTracefp != NULL) fclose(VaccineTracefp);
  time(&clock);
  fprintf(Statusfp, "FRED finished %s", ctime(&clock));
  return 0;
}

