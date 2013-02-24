/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
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
#include "Small_Grid.h"
#include "Params.h"
#include "Random.h"
#include "Vaccine_Manager.h"
#include "Date.h"
#include "Evolution.h"
#include "Travel.h"
#include "Epidemic.h"
#include "Seasonality.h"
#include "Past_Infection.h"
#include "DB.h"
#include "Activities.h"
#include "Behavior.h"

#include "execinfo.h"
#include <csignal>
#include <cstdlib>
#include <cxxabi.h>

int main(int argc, char* argv[]) {

  int run;          // number of current run
  unsigned long new_seed;
  char directory[FRED_STRING_SIZE];
  char paramfile[FRED_STRING_SIZE];

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

  Global::db.open_database( Global::DBfile );

  // get runtime population parameters
  Global::Pop.get_parameters();

  // initialize masks in Global::Pop
  Global::Pop.initialize_masks();

  if (strcmp(directory, "") == 0) {
    // use the directory in the params file
    strcpy(directory, Global::Output_directory);
    //printf("directory = %s\n",directory);
  } 
  else {
    // change the Output_directory
    strcpy(Global::Output_directory, directory);
    FRED_STATUS( 0, "Overridden from command line: Output_directory = %s\n",
        Global::Output_directory );
  }

  // create the output directory, if necessary
  mode_t mask;        // the user's current umask
  mode_t mode = 0777; // as a start
  mask = umask(0); // get the current mask, which reads and sets...
  umask(mask);     // so now we have to put it back
  mode ^= mask;    // apply the user's existing umask
  if (0!=mkdir(directory, mode) && EEXIST!=errno) // make it
    Utils::fred_abort("mkdir(Output_directory) failed with %d\n",errno); // or die

  // open output files with global file pointers
  Utils::fred_open_output_files(directory, run, mode);

  // initialize RNG
  INIT_RANDOM(Global::Seed);

  // Date Setup
  // Start_date parameter must have format 'YYYY-MM-DD'
  Global::Sim_Start_Date = new Date(string(Global::Start_date));
  Global::Sim_Current_Date = new Date(string(Global::Start_date));

  if (Global::Rotate_start_date) {
    // add one day to the start date for each additional run,
    // rotating the days of the week to reduce weekend effect.
    Global::Sim_Start_Date->advance((run-1)%7);
    Global::Sim_Current_Date->advance((run-1)%7);
  }

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

  // Initializes Synthetic Population parameters, determines the synthetic
  // population id if the city or county was specified as a parameter
  // Must be called BEFORE Pop.split_synthetic_populations_by_deme() because
  // city/county population lookup may overwrite Global::Synthetic_population_id
  Global::Places.get_parameters();

  // split the population id parameter string ( that was initialized in 
  // Places::get_parameters ) on whitespace; each population id is processed as a
  // separate deme, and stored in the Population object.
  Global::Pop.split_synthetic_populations_by_deme();

  // Loop over all Demes and read in the household, schools and workplaces
  // and setup grids and cells
  Global::Places.read_all_places( Global::Pop.get_demes() );
  Utils::fred_print_lap_time("Places.read_places");

  // initialize activities
  Activities::read_init_files();
  // initialize behaviors
  Behavior::init_static_params();

  // read in the population and have each person enroll
  // in each favorite place identified in the population file
  Global::Pop.setup();
  Utils::fred_print_lap_time("Pop.setup");

  // define FRED-specific places and have each person enroll as needed

  // classrooms
  Global::Places.setup_classrooms();
  Global::Pop.assign_classrooms();
  Utils::fred_print_lap_time("assign classrooms");

  // teachers
  if (Global::Assign_Teachers) {
    Global::Places.assign_teachers();
    Utils::fred_print_lap_time("assign teachers");
  }

  // offices
  Global::Places.setup_offices();
  Global::Pop.assign_offices();
  Utils::fred_print_lap_time("assign offices");

  // after all enrollments, prepare to receive visitors
  Global::Places.prepare();

  // record the favorite places for households within each grid cell
  Global::Cells->record_favorite_places();
  Utils::fred_print_lap_time("place prep");

  if (Global::Enable_Travel) {
    Global::Large_Cells->set_population_size();
    Travel::setup(directory);
    Utils::fred_print_lap_time("Travel setup");
  }

  if (Global::Quality_control) {
    Global::Pop.quality_control();
    Global::Places.quality_control(directory);
    Global::Large_Cells->quality_control(directory);
    Global::Cells->quality_control(directory);
    if (Global::Enable_Small_Grid) {
      Global::Small_Cells->quality_control(directory);
    }
    if (Global::Track_network_stats) {
      Global::Pop.get_network_stats(directory);
    }
    Utils::fred_print_lap_time("quality control");
  }

  if (Global::Track_age_distribution) {
    Global::Pop.print_age_distribution(directory,
        (char *) Global::Sim_Start_Date->get_YYYYMMDD().c_str(), run);
  }

  if (Global::Enable_Seasonality) {
    Global::Clim->print_summary();
  }

  for (int d = 0; d < Global::Diseases; ++d) {
    Disease * disease = Global::Pop.get_disease( d );
    disease->initialize_evolution_reporting_grid( Global::Large_Cells );
    disease->init_prior_immunity();
  }

  // initialize GAIA data if desired
  if (Global::Print_GAIA_Data) {
    Global::Small_Cells->initialize_gaia_data(directory, run);
  }

  Utils::fred_print_lap_time("FRED initialization");
  Utils::fred_print_wall_time("FRED initialization complete");

  time_t simulation_start_time;
  Utils::fred_start_timer( &simulation_start_time );

  for (int day = 0; day < Global::Days; day++) {

    Utils::fred_start_day_timer();
    if (day == Global::Reseed_day) {
      fprintf(Global::Statusfp, "************** reseed day = %d\n", day);
      fflush(Global::Statusfp);
      INIT_RANDOM(new_seed + run - 1);
    }

    if ( Date::match_pattern( Global::Sim_Current_Date, "01-01-*" ) ) {
      if ( Global::Track_age_distribution ) {
        Global::Pop.print_age_distribution(directory,
            (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str(), run);
        Global::Places.print_household_size_distribution(directory,
            (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str(), run);
      }
      if ( Global::Track_household_distribution ) {
        Global::Cells->print_household_distribution(directory,
            (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str(), run);
      }
    }

    if (Global::Enable_Small_Grid)
      Global::Small_Cells->update(day);

    Global::Places.update(day);
    Utils::fred_print_lap_time("day %d update places", day);

    Global::Pop.update(day);
    Utils::fred_print_lap_time("day %d update population", day);

    Epidemic::update(day);
    Utils::fred_print_lap_time("day %d update epidemics", day);

    Global::Pop.report(day);
    Utils::fred_print_lap_time("day %d report population", day);

    if ( Global::Enable_Migration
        && Date::match_pattern( Global::Sim_Current_Date, "02-*-*" ) ) {
      Global::Cells->population_migration( day );
    }
    
    if ( Global::Enable_Aging && Global::Verbose
        && Date::match_pattern( Global::Sim_Current_Date, "12-31-*" ) ) {
      Global::Pop.quality_control();
    }

    if (Date::match_pattern(Global::Sim_Current_Date, "01-01-*")) {
      if (Global::Track_age_distribution) {
        Global::Pop.print_age_distribution(directory,
            (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str(), run);
        Global::Places.print_household_size_distribution(directory,
            (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str(), run);
      }
      if (Global::Track_household_distribution) {
        Global::Cells->print_household_distribution(directory,
            (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str(), run);
      }
    }

    // incremental trace
    if (Global::Incremental_Trace && day && !(day%Global::Incremental_Trace))
      Global::Pop.print(1, day);

    // print GAIA data if desired
    if (Global::Print_GAIA_Data) {
      Global::Places.report(day);
      Global::Small_Cells->print_gaia_data(directory, run, day);
    }

    #pragma omp parallel sections
    {
      #pragma omp section
      {
        // this refreshes all RNG buffers in a new thread team
        RNG::refresh_all_buffers();
      }
      #pragma omp section
      {
        // flush infections file buffer
        fflush(Global::Infectionfp);
      }
      #pragma omp section
      {
        Global::db.process_transactions();
      }
    }
 
    Utils::fred_print_wall_time("day %d finished", day);

    Utils::fred_print_day_timer(day);
    Utils::fred_print_resource_usage(day);

    Global::Sim_Current_Date->advance();
  }
 
  Global::db.close_database();

  fflush(Global::Infectionfp);

  Utils::fred_print_lap_time( &simulation_start_time,
      "\nFRED simulation complete. Excluding initialization, %d days",
      Global::Days);

  Utils::fred_print_wall_time("FRED finished");
  Utils::fred_print_finish_timer();

  // finish up
  Global::Pop.end_of_run();
  Global::Places.end_of_run();

  // close all open output files with global file pointers
  Utils::fred_end();

  return 0;
}
