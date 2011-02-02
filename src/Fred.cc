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
#include "Locations.h"
#include "Params.h"
#include "Random.h"
#include "Vaccine_Manager.h"
#include "Date.h"

char Paramfile[80];
Date * Fred_Date;

int main(int argc, char* argv[]) {
  time_t clock;					// current date
  int single_run_number;			// number of single run
  if (argc > 2) {
    sscanf(argv[2], "%d", &single_run_number);
  }
  else {
    single_run_number = 0;
  }
  if (argc > 1) {
    strcpy(Paramfile, argv[1]);
  }
  else {
    strcpy(Paramfile, "params");
  }
	
  Statusfp = stdout;
  time(&clock);
  fprintf(Statusfp, "FRED started %s", ctime(&clock));
  fprintf(Statusfp, "param file = %s\n", Paramfile);
  fflush(Statusfp);
       
  setup(Paramfile);

  if (single_run_number > 0) {
    run_sim(single_run_number);
  }
  else {
    for (int run = 1; run <= Runs; run++) {
      run_sim(run);
    }
  }
  time(&clock);
  fprintf(Statusfp, "FRED finished %s", ctime(&clock));
  return 0;
}

void setup(char *paramfile) {
  mode_t mask;        // the user's current umask
  mode_t mode = 0777; // as a start
	
  read_parameters(paramfile);
  get_global_parameters();

  // create the output directory, if it does not already exist
  mask = umask(0); // get the current mask, which reads and sets...
  umask(mask);     // so now we have to put it back
  mode ^= mask;    // apply the user's existing umask
  if (0!=mkdir(Output_directory, mode) && EEXIST!=errno) // make it
    err(errno, "mkdir(Output_directory) failed");      // or die

  Random_start_day = (Epidemic_offset > 6);
  Pop.get_parameters();
  Loc.get_location_parameters();
  Loc.setup_locations();
  Pop.setup();

  // Date Setup
  // If Start_date parameter is "today", then set do default constructor
  if (strncmp(Start_date, "today", 5)==0) {
    Fred_Date = new Date(Days);

  } else {
    int day_of_month = Date::parse_day_of_month_from_date_string(string(Start_date), Date::MMDDYYYY);
    int month = Date::parse_month_from_date_string(string(Start_date), Date::MMDDYYYY);
    int year = Date::parse_year_from_date_string(string(Start_date), Date::MMDDYYYY);

    Fred_Date = new Date(year, month, day_of_month, Days);

  }

  // Write the date info to a file
  char filename[256];
  sprintf(filename, "%s/fred_date.txt", Output_directory);
  FredDatefp = fopen(filename, "w");
  if (FredDatefp == NULL) {
    printf("Help! Can't open %s\n", filename);
    abort();
  }

  for (int day = 0; day < Days; day++) {
    fprintf(FredDatefp, "%d\t%s %d/%d/%d\t%d Week %d\n",
      day,
      Fred_Date->get_day_of_week_string(day).c_str(),
      Fred_Date->get_month(day),
      Fred_Date->get_day_of_month(day),
      Fred_Date->get_year(day),
      Fred_Date->get_week_year(day),
      Fred_Date->get_week(day));
  }

  fclose(FredDatefp);

  if (Quality_control) {
    Pop.population_quality_control();
    Loc.location_quality_control();
  }
}

void run_sim(int run) {
  char filename[256];
  unsigned long new_seed;
  time_t clock;		      // current date
	
  sprintf(filename, "%s/out%d.txt", Output_directory, run);
  Outfp = fopen(filename, "w");
  if (Outfp == NULL) {
    printf("Help! Can't open %s\n", filename);
    abort();
  }
	
  Tracefp = NULL;
  if (strcmp(Tracefilebase, "none") != 0) {
    sprintf(filename, "%s/trace%d.txt", Output_directory, run);
    Tracefp = fopen(filename, "w");
    if (Tracefp == NULL) {
      printf("Help! Can't open %s\n", filename);
      abort();
    }
  }
	
  VaccineTracefp = NULL;
  if (strcmp(VaccineTracefilebase, "none") != 0) {
    sprintf(filename, "%s/vacctr%d.txt", Output_directory, run);
    VaccineTracefp = fopen(filename, "w");
    if (VaccineTracefp == NULL) {
      printf("Help! Can't open %s\n", filename);
      abort();
    }
  }
	
  time(&clock);
  fprintf(Statusfp, "\nFRED run %d started %s\n", run, ctime(&clock));
  fflush(Statusfp);
	
  // allow us to replicate individual runs
  if (run > 1 && Reseed_day == -1) { new_seed = Seed * 100 + (run-1); }
  else { new_seed = Seed; }
  fprintf(Statusfp, "seed = %lu\n", new_seed);
  INIT_RANDOM(new_seed);
  Loc.reset_locations(run);
  Pop.reset(run);
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
    Loc.update(day);
    Pop.update(day);
    Pop.report(day);
    fprintf(Statusfp, "day %d finished  ", day);
    // incremental trace
    if (Incremental_Trace && day && !(day%Incremental_Trace))
      Pop.print(1, day);
    time(&clock);
    fprintf(Statusfp, "%s", ctime(&clock));

  }
  Pop.end_of_run();
  fclose(Outfp);
  if (Tracefp != NULL) fclose(Tracefp);
}

void cleanup(int run) {

}
