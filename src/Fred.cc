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
#include "Large_Grid.h"
#include "Params.h"
#include "Random.h"
#include "Vaccine_Manager.h"
#include "Date.h"
#include "Evolution.h"
#include "Travel.h"

int main(int argc, char* argv[]) {
  time_t start_timer, stop_timer, fred_timer;
  int run;          // number of current run
  unsigned long new_seed;
  char filename[256];
  char directory[256];
  char paramfile[256];

  time(&fred_timer);

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
  time(&start_timer);
  fprintf(Global::Statusfp, "FRED started %s", ctime(&start_timer));
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
  // STB Do the error file first so that it captures
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

  // Date Setup
  INIT_RANDOM(Global::Seed);
  Global::Random_start_day = (Global::Epidemic_offset > 6);

  // Start_date must have format 'YYYY-MM-DD'
  Global::Sim_Date = new Date(string(Global::Start_date));
  Global::Sim_Date->setup(directory, Global::Days);

  // set random number seed based on run number
  if (run > 1 && Global::Reseed_day == -1) {
    new_seed = Global::Seed * 100 + (run-1);
  } else {
    new_seed = Global::Seed;
  }
  fprintf(Global::Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);

  time(&start_timer);
  fprintf(Global::Statusfp, "\nFRED run %d started %s\n", run, ctime(&start_timer));
  fflush(Global::Statusfp);

  // initializations

  // read in the household, schools and workplaces (also sets up grids)
  time(&start_timer);
  Global::Places.read_places();
  time(&stop_timer);
  fprintf(Global::Statusfp, "Places.read_places() took %d seconds\n",
	  (int) (stop_timer - start_timer));
  fflush(Global::Statusfp);

  // read in the population and have each person enroll
  // in each favorite place identified in the population file
  time(&start_timer);
  Global::Pop.setup();
  time(&stop_timer);
  fprintf(Global::Statusfp, "Pop.setup() took %d seconds\n",
	  (int) (stop_timer - start_timer));
  fflush(Global::Statusfp);

  // define FRED-specific places
  // and have each person enroll as needed
  time(&start_timer);
  Global::Places.setup_classrooms();
  Global::Places.setup_offices();
  Global::Pop.assign_classrooms();
  Global::Pop.assign_offices();
  time(&stop_timer);
  fprintf(Global::Statusfp, "assign classrooms and offices took %d seconds\n",
	  (int) (stop_timer - start_timer));
  fflush(Global::Statusfp);

  // after all enrollments, prepare to receive visitors
  time(&start_timer);
  Global::Places.prepare();

  // record the favorite places for households within each grid cell
  Global::Cells->record_favorite_places();
  time(&stop_timer);
  fprintf(Global::Statusfp, "place prep took %d seconds\n",
	  (int) (stop_timer - start_timer));
  fflush(Global::Statusfp);

  if (Global::Enable_Large_Grid && Global::Enable_Travel) {
    time(&start_timer);
    Travel::setup();
    time(&stop_timer);
    fprintf(Global::Statusfp, "Travel setup took %d seconds\n",
	    (int) (stop_timer - start_timer));
    fflush(Global::Statusfp);
  }

  if (Global::Quality_control) {
    time(&start_timer);
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
    Global::Pop.get_network_stats(directory);
    time(&stop_timer);
    fprintf(Global::Statusfp, "quality control took %d seconds\n",
	    (int) (stop_timer - start_timer));
    fflush(Global::Statusfp);
  }

  // allow for an offset in the start of the epidemic
  if (Global::Random_start_day) {
    // cycle through days of the week for start day
    Global::Epidemic_offset = (run-1) % 7;
  }

  if (Global::Track_age_distribution) {
    Global::Pop.print_age_distribution(directory, Global::Sim_Date->get_YYYYMMDD(0), run);
  }

  stop_timer = time(&stop_timer);
  fprintf(Global::Statusfp, "FRED initializations complete at %s\n",
	  ctime(&stop_timer));
  fprintf(Global::Statusfp, "FRED initializations took %d secs\n",
	  (int) (stop_timer-start_timer));
  start_timer = stop_timer;
  fflush(Global::Statusfp);

  for (int day = 0; day < Global::Days; day++) {
    time_t day_timer;
    time(&day_timer);
    if (day == Global::Reseed_day) {
      fprintf(Global::Statusfp, "************** reseed day = %d\n", day);
      fflush(Global::Statusfp);
      INIT_RANDOM(new_seed + run - 1);
    }
    time(&start_timer);

    Global::Places.update(day);
    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d  Places.update took %d seconds\n",
	    day,(int)(stop_timer-start_timer));
    fflush(Global::Statusfp);
    start_timer = stop_timer;

    Global::Pop.begin_day(day);
    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d  Pop.begin took %d seconds\n",
	    day,(int)(stop_timer-start_timer));
    fflush(Global::Statusfp);
    start_timer = stop_timer;

    Global::Pop.get_visitors_to_infectious_places(day);
    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d  Pop.get_visitors took %d seconds\n",
	    day,(int)(stop_timer-start_timer));
    fflush(Global::Statusfp);
    start_timer = stop_timer;

    Global::Pop.transmit_infection(day);
    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d  Pop.transmit took %d seconds\n",
	    day,(int)(stop_timer-start_timer));
    fflush(Global::Statusfp);
    start_timer = stop_timer;

    Global::Pop.end_day(day);
    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d  Pop.end_day took %d seconds\n",
	    day,(int)(stop_timer-start_timer));
    fflush(Global::Statusfp);
    start_timer = stop_timer;

    Global::Pop.report(day);
    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d  Pop.report took %d seconds\n",
	    day,(int)(stop_timer-start_timer));
    fflush(Global::Statusfp);
    start_timer = stop_timer;

    if (Global::Enable_Migration && Date::match_pattern(day, "02-*-*")) {
      Global::Cells->population_migration(day);
    }
    
    if (Global::Enable_Aging && Global::Verbose && Date::match_pattern(day, "12-31-*")) {
      Global::Pop.quality_control();
      Global::Places.quality_control(directory);
      Global::Cells->quality_control(directory);
    }

    if (Date::match_pattern(day,"01-01-*")) {
      if (Global::Track_age_distribution) {
	Global::Pop.print_age_distribution(directory, Global::Sim_Date->get_YYYYMMDD(day), run);
      }
      if (Global::Track_household_distribution) {
	Global::Cells->print_household_distribution(directory, Global::Sim_Date->get_YYYYMMDD(day), run);
      }
    }

    // incremental trace
    if (Global::Incremental_Trace && day && !(day%Global::Incremental_Trace))
      Global::Pop.print(1, day);

    time(&stop_timer);
    fprintf(Global::Statusfp, "day %d finished  %s", day, ctime(&stop_timer));
    fprintf(Global::Statusfp, "day %d took %d seconds\n",
	    day,(int)(stop_timer-day_timer));
    fflush(Global::Statusfp);
  }

  time(&stop_timer);
  fprintf(Global::Statusfp, "FRED finished %s", ctime(&stop_timer));
  fprintf(Global::Statusfp, "FRED took %d seconds\n",
	  (int)(stop_timer-fred_timer));
  fflush(Global::Statusfp);

  // finish up
  Global::Pop.end_of_run();
  Utils::fred_end();

  return 0;
}

