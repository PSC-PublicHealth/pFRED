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
#include "Evolution.h"

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
  Infectionfp = NULL;
  if (Track_infection_events) {
    sprintf(filename, "%s/infections%d.txt", directory, run);
    Infectionfp = fopen(filename, "w");
    if (Infectionfp == NULL) {
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
  Birthfp = NULL;
  if (Enable_Births) {
    sprintf(filename, "%s/births%d.txt", directory, run);
    Birthfp = fopen(filename, "w");
    if (Birthfp == NULL) {
      printf("Help! Can't open %s\n", filename); abort();
    }
  }
  Deathfp = NULL;
  if (Enable_Deaths) {
    sprintf(filename, "%s/deaths%d.txt", directory, run);
    Deathfp = fopen(filename, "w");
    if (Deathfp == NULL) {
      printf("Help! Can't open %s\n", filename); abort();
    }
  }
  Prevfp = NULL;
  if (strcmp(Prevfilebase, "none") != 0) {
    sprintf(filename, "%s/prev%d.txt", Output_directory, run);
    Prevfp = fopen(filename, "w");
    if (Prevfp == NULL) {
      printf("Help! Can't open %s\n", filename);
      abort();
    }
  }
  Incfp = NULL;
  if (strcmp(Incfilebase, "none") != 0) {
    sprintf(filename, "%s/inc%d.txt", Output_directory, run);
    Incfp = fopen(filename, "w");
    if (Incfp == NULL) {
      printf("Help! Can't open %s\n", filename);
      abort();
    }
  }

  // Date Setup
  INIT_RANDOM(Seed);
  Random_start_day = (Epidemic_offset > 6);

  // Start_date must have format 'YYYY-MM-DD'
  Sim_Date = new Date(string(Start_date), Date::YYYYMMDD);
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
      fprintf(Statusfp, "************** reseed day = %d\n", day);
      fflush(Statusfp);
      INIT_RANDOM(new_seed + run - 1);
    }
    // fprintf(Statusfp, "================\nsim day = %d  date = %s\n",
    // day, Sim_Date->get_MMDD(day));
    // fflush(stdout);
    Places.update(day);
    Pop.begin_day(day);
    Pop.get_visitors_to_infectious_places(day);
    Pop.transmit_infection(day);
    Pop.end_day(day);
    Pop.report(day);

    if (Enable_Aging && Verbose && strcmp(Sim_Date->get_MMDD(day),"12-31")==0) {
      Pop.quality_control();
      Places.quality_control();
      Environment.quality_control();
    }

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
  if (Infectionfp != NULL) fclose(Infectionfp);
  if (VaccineTracefp != NULL) fclose(VaccineTracefp);
  if (Prevfp != NULL) fclose(Prevfp);
  if (Incfp != NULL) fclose(Incfp);
  time(&clock);
  fprintf(Statusfp, "FRED finished %s", ctime(&clock));
  return 0;
}

