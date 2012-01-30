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
#include "Population.h"
#include "Place_List.h"
#include "Grid.h"
#include "Large_Grid.h"
#include "Params.h"
#include "Random.h"
#include "Vaccine_Manager.h"
#include "Date.h"
#include "Evolution.h"
#include "Travel.h"
#include "Epidemic.h"
#include "Seasonality.h"

int main(int argc, char* argv[]) {
  int run;          // number of current run
  unsigned long new_seed;
  char filename[256];
  char directory[256];
  char paramfile[256];

  Global::Statusfp = stdout;
  Utils::fred_start_timer();
  Utils::fred_print_wall_time("FRED started");

  // read optional param file name from command line
  if (argc > 1) {
    strcpy(paramfile, argv[1]);
  } else {
    strcpy(paramfile, "params");
  }
  fprintf(Global::Statusfp, "param file = %s\n", paramfile);
  fflush(Global::Statusfp);

  // read optional run number from command line (must be 2nd arg)
  if (argc > 2) {
    sscanf(argv[2], "%d", &run);
  } else {
    run = 1;
  }

  // read optional working directory from command line (must be 3rd arg)
  if (argc > 3) {
    strcpy(directory, argv[3]);
  } else {
    strcpy(directory, "");
  }

  // get runtime parameters
  Params::read_parameters(paramfile);
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
  // Do the error file first so that it captures
  // as much errors as possible.
  Global::ErrorLogfp = NULL;
  sprintf(filename, "%s/err%d.txt", directory, run);
  Global::ErrorLogfp = fopen(filename, "w");
  if (Global::ErrorLogfp == NULL) {
    Utils::fred_abort("Can't open %s\n", filename);
  }
  sprintf(filename, "%s/out%d.txt", directory, run);
  Global::Outfp = fopen(filename, "w");
  if (Global::Outfp == NULL) {
    Utils::fred_abort("Can't open %s\n", filename);
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
    sprintf(filename, "%s/prev%d.txt", directory, run);
    Global::Prevfp = fopen(filename, "w");
    if (Global::Prevfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Incfp = NULL;
  if (strcmp(Global::Incfilebase, "none") != 0) {
    sprintf(filename, "%s/inc%d.txt", directory, run);
    Global::Incfp = fopen(filename, "w");
    if (Global::Incfp == NULL) {
      Utils::fred_abort("Help! Can't open %s\n", filename);
    }
  }

  // initialize RNG
  INIT_RANDOM(Global::Seed);

  // Date Setup
  // Start_date parameter must have format 'YYYY-MM-DD'
  Global::Sim_Date = new Date(string(Global::Start_date));

  if (Global::Rotate_start_date) {
    // add one day to the start date for each additional run,
    // rotating the days of the week to reduce weekend effect.
    Global::Sim_Date->advance((run-1)%7);
  }

  Global::Sim_Date->setup(directory, Global::Days);

  // set random number seed based on run number
  if (run > 1 && Global::Reseed_day == -1) {
    new_seed = Global::Seed * 100 + (run-1);
  } else {
    new_seed = Global::Seed;
  }
  fprintf(Global::Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);

  Utils::fred_print_wall_time("\nFRED run %d started", (int) run);

  // initializations

  // read in the household, schools and workplaces (also sets up grids)
  Global::Places.read_places();
  Utils::fred_print_lap_time("Places.read_places");

  // read in the population and have each person enroll
  // in each favorite place identified in the population file
  Global::Pop.setup();
  Utils::fred_print_lap_time("Pop.setup");

  // define FRED-specific places
  // and have each person enroll as needed
  Global::Places.setup_classrooms();
  Global::Places.setup_offices();
  Global::Pop.assign_classrooms();
  Global::Pop.assign_offices();
  Utils::fred_print_lap_time("assign classrooms and offices");

  // after all enrollments, prepare to receive visitors
  Global::Places.prepare();

  // record the favorite places for households within each grid cell
  Global::Cells->record_favorite_places();
  Utils::fred_print_lap_time("place prep");

  if (Global::Enable_Large_Grid && Global::Enable_Travel) {
    Global::Large_Cells->set_population_size();
    Travel::setup(directory);
    Utils::fred_print_lap_time("Travel setup");
  }

  if (Global::Quality_control) {
    Global::Pop.quality_control();
    Global::Places.quality_control(directory);
    if (Global::Enable_Large_Grid) {
      Global::Large_Cells->quality_control(directory);
      // get coordinates of large grid as alinged to global grid
      double min_x = Global::Large_Cells->get_min_x();
      double min_y = Global::Large_Cells->get_min_y();
      Global::Cells->quality_control(directory,min_x, min_y);
    }
    else {
      Global::Cells->quality_control(directory);
    }
    if (Global::Track_network_stats) 
      Global::Pop.get_network_stats(directory);
    Utils::fred_print_lap_time("quality control");
  }

  if (Global::Track_age_distribution) {
    Global::Pop.print_age_distribution(directory, (char *) Global::Sim_Date->get_YYYYMMDD(0).c_str(), run);
  }

  if (Global::Enable_Seasonality) {
    Global::Clim->print_summary();
  }

  Utils::fred_print_lap_time("FRED initialization");
  Utils::fred_print_wall_time("FRED initialization complete");

  for (int day = 0; day < Global::Days; day++) {
    Utils::fred_start_day_timer();
    if (day == Global::Reseed_day) {
      fprintf(Global::Statusfp, "************** reseed day = %d\n", day);
      fflush(Global::Statusfp);
      INIT_RANDOM(new_seed + run - 1);
    }

    Global::Places.update(day);
    Utils::fred_print_lap_time("day %d update places", day);

    Global::Pop.update(day);
    Utils::fred_print_lap_time("day %d update population", day);

    Epidemic::update(day);
    Utils::fred_print_lap_time("day %d update epidemics", day);

    Global::Pop.report(day);
    Utils::fred_print_lap_time("day %d report population", day);

    if (Global::Enable_Migration && Date::match_pattern(day, "02-*-*")) {
      Global::Cells->population_migration(day);
    }
    
    if (Global::Enable_Aging && Global::Verbose && Date::match_pattern(day, "12-31-*")) {
      Global::Pop.quality_control();
    }

    if (Date::match_pattern(day,"01-01-*")) {
      if (Global::Track_age_distribution) {
	Global::Pop.print_age_distribution(directory, (char *) Global::Sim_Date->get_YYYYMMDD(day).c_str(), run);
      }
      if (Global::Track_household_distribution) {
	Global::Cells->print_household_distribution(directory, (char *) Global::Sim_Date->get_YYYYMMDD(day).c_str(), run);
      }
    }

    // incremental trace
    if (Global::Incremental_Trace && day && !(day%Global::Incremental_Trace))
      Global::Pop.print(1, day);

    Utils::fred_print_wall_time("day %d finished", day);

    Utils::fred_print_day_timer(day);
  }

  Utils::fred_print_wall_time("FRED finished");
  Utils::fred_print_finish_timer();

  // finish up
  Global::Pop.end_of_run();
  Global::Places.end_of_run();
  Utils::fred_end();

  return 0;
}

