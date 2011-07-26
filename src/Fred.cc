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
#include "Utils.h"
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


int main(int argc, char* argv[]) {
  time_t clock;         // current date
  int run;          // number of current run
  unsigned long new_seed;
  char filename[256];
  char directory[256];
  char paramfile[256];

  if (argc > 1) {
    strcpy(paramfile, argv[1]);
  } else {
    strcpy(paramfile, "params");
  }

  if (argc > 2) {
    sscanf(argv[2], "%d", &run);
  } else {
    run = 1;
  }

  if (argc > 3) {
    strcpy(directory, argv[3]);
  } else {
    strcpy(directory, "");
  }

  Global::Statusfp = stdout;
  time(&clock);
  fprintf(Global::Statusfp, "FRED started %s", ctime(&clock));
  fprintf(Global::Statusfp, "param file = %s\n", paramfile);
  fflush(Global::Statusfp);


  // get runtime parameters
  read_parameters(paramfile);
  Global::get_global_parameters();

  // get runtime population parameters
  Global::Pop.get_parameters();

  if (strcmp(directory, "") == 0) {
    // use the directory in the params file
    strcpy(directory, Global::Output_directory);
    //printf("directory = %s\n",directory);
  }
  // create the output directory, if necessary
  mode_t mask;        // the user's current umask
  mode_t mode = 0777; // as a start
  mask = umask(0); // get the current mask, which reads and sets...
  umask(mask);     // so now we have to put it back
  mode ^= mask;    // apply the user's existing umask
  if (0!=mkdir(directory, mode) && EEXIST!=errno) // make it
    Utils::fred_abort("mkdir(Output_directory) failed with %d\n",errno); // or die

  // open output files
  sprintf(filename, "%s/out%d.txt", directory, run);
  Global::Outfp = fopen(filename, "w");
  if (Global::Outfp == NULL) {
    Utils::fred_abort("Can't open %s\n", filename);
  }
  // STB Do the error file first so that it captures
  // as much errors as possible.
  Global::ErrorLogfp = NULL;
  sprintf(filename, "%s/err%d.txt", Global::Output_directory, run);
  Global::ErrorLogfp = fopen(filename, "w");
  if (Global::ErrorLogfp == NULL) {
    printf("Help! Can't open %s\n", filename);
    abort();
  }
  Global::Tracefp = NULL;
  if (strcmp(Global::Tracefilebase, "none") != 0) {
    sprintf(filename, "%s/trace%d.txt", directory, run);
    Global::Tracefp = fopen(filename, "w");
    if (Global::Tracefp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Infectionfp = NULL;
  if (Global::Track_infection_events) {
    sprintf(filename, "%s/infections%d.txt", directory, run);
    Global::Infectionfp = fopen(filename, "w");
    if (Global::Infectionfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::VaccineTracefp = NULL;
  if (strcmp(Global::VaccineTracefilebase, "none") != 0) {
    sprintf(filename, "%s/vacctr%d.txt", directory, run);
    Global::VaccineTracefp = fopen(filename, "w");
    if (Global::VaccineTracefp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Birthfp = NULL;
  if (Global::Enable_Births) {
    sprintf(filename, "%s/births%d.txt", directory, run);
    Global::Birthfp = fopen(filename, "w");
    if (Global::Birthfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Deathfp = NULL;
  if (Global::Enable_Deaths) {
    sprintf(filename, "%s/deaths%d.txt", directory, run);
    Global::Deathfp = fopen(filename, "w");
    if (Global::Deathfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Prevfp = NULL;
  if (strcmp(Global::Prevfilebase, "none") != 0) {
    sprintf(filename, "%s/prev%d.txt", Global::Output_directory, run);
    Global::Prevfp = fopen(filename, "w");
    if (Global::Prevfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Incfp = NULL;
  if (strcmp(Global::Incfilebase, "none") != 0) {
    sprintf(filename, "%s/inc%d.txt", Global::Output_directory, run);
    Global::Incfp = fopen(filename, "w");
    if (Global::Incfp == NULL) {
      Utils::fred_abort("Help! Can't open %s\n", filename);
    }
  }


  // Date Setup
  INIT_RANDOM(Global::Seed);
  Global::Random_start_day = (Global::Epidemic_offset > 6);

  // Start_date must have format 'YYYY-MM-DD'
  Global::Sim_Date = new Date(string(Global::Start_date), Date::YYYYMMDD);
  Global::Sim_Date->setup(directory, Global::Days);

  // set random number seed based on run number
  if (run > 1 && Global::Reseed_day == -1) {
    new_seed = Global::Seed * 100 + (run-1);
  } else {
    new_seed = Global::Seed;
  }
  fprintf(Global::Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);

  time(&clock);
  fprintf(Global::Statusfp, "\nFRED run %d started %s\n", run, ctime(&clock));
  fflush(Global::Statusfp);

  // initializations

  // read in the household, schools and workplaces (also sets up grids)
  Global::Places.read_places();

  // read in the population and have each person enroll
  // in each favorite place identified in the population file
  Global::Pop.setup();

  // define FRED-specific places
  Global::Places.setup_classrooms();
  Global::Places.setup_offices();

  // define FRED-specific social networks,
  // and have each person enroll as needed
  Global::Pop.assign_classrooms();
  Global::Pop.assign_offices();

  // after all enrollments, prepare to receive visitors
  Global::Places.prepare();

  // record the favorite places for households within each grid_cell
  Global::Cells->record_favorite_places();

  if (Global::Quality_control) {
    Global::Pop.quality_control();
    Global::Places.quality_control();
    Global::Cells->quality_control();
  }

  // allow for an offset in the start of the epidemic
  if (Global::Random_start_day) {
    // cycle through days of the week for start day
    Global::Epidemic_offset = (run-1) % 7;
  }

  if (Global::Track_age_distribution) {
    Global::Pop.print_age_distribution(directory, Global::Sim_Date->get_YYYYMMDD(0), run);
  }

  for (int day = 0; day < Global::Days; day++) {
    if (day == Global::Reseed_day) {
      fprintf(Global::Statusfp, "************** reseed day = %d\n", day);
      fflush(Global::Statusfp);
      INIT_RANDOM(new_seed + run - 1);
    }
    // fprintf(Statusfp, "================\nsim day = %d  date = %s\n",
    // day, Global::Sim_Date->get_MMDD(day));
    // fflush(stdout);

    Global::Places.update(day);
    Global::Pop.begin_day(day);
    Global::Pop.get_visitors_to_infectious_places(day);
    Global::Pop.transmit_infection(day);
    Global::Pop.end_day(day);
    Global::Pop.report(day);
    
    char date_string[10];
    strcpy(date_string, Global::Sim_Date->get_MMDD(day));
    if (Global::Enable_Migration && (date_string[3] == '0' && date_string[4] == '2')) {
      printf("MIGRATION called on %s\n", date_string); fflush(stdout);
      Global::Cells->population_migration();
    }
    
    if (Global::Enable_Aging && Global::Verbose && strcmp(Global::Sim_Date->get_MMDD(day),"12-31")==0) {
      Global::Pop.quality_control();
      Global::Places.quality_control();
      Global::Cells->quality_control();
    }

    if (Global::Track_age_distribution && (strcmp(Global::Sim_Date->get_MMDD(day),"01-01")==0)) {
      Global::Pop.print_age_distribution(directory, Global::Sim_Date->get_YYYYMMDD(day), run);
    }

    if (Global::Track_household_distribution && (strcmp(Global::Sim_Date->get_MMDD(day),"01-01")==0)) {
      Global::Cells->print_household_distribution(directory, Global::Sim_Date->get_YYYYMMDD(day), run);
    }

    fprintf(Global::Statusfp, "day %d finished  ", day);
    // incremental trace
    if (Global::Incremental_Trace && day && !(day%Global::Incremental_Trace))
      Global::Pop.print(1, day);
    time(&clock);
    fprintf(Global::Statusfp, "%s", ctime(&clock));
  }


  // finish up
  Global::Pop.end_of_run();
  // fclose(Statusfp);
  Utils::fred_end();
  time(&clock);
  fprintf(Global::Statusfp, "FRED finished %s", ctime(&clock));
  return 0;
}

