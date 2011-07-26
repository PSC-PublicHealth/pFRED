//
//
// File: Global.cc
//

#include "Global.h"
#include "Params.h"
#include "Population.h"
#include "Place_List.h"
#include "Grid.h"
#include "Large_grid.h"
#include "Small_grid.h"

// global runtime parameters
char Global::Population_directory[256];
char Global::Output_directory[256];
char Global::Tracefilebase[256];
char Global::Eventfilebase[256];
char Global::VaccineTracefilebase[256];
int Global::Incremental_Trace = 0;
int Global::Trace_Headers = 0;
int Global::Random_start_day = 0;
char Global::Behavior_model_type[256];
int Global::Quality_control = 0;
int Global::Diseases = 0;
double Global::Prob_accept_vaccine = 0.0;
double Global::Prob_accept_vaccine_dose = 0.0;
int Global::StrainEvolution = 0;
char Global::Prevfilebase[256];
char Global::Incfilebase[256];
char Global::ErrorLogbase[256];
int Global::Enable_Large_grid = 0;
int Global::Enable_Small_grid = 0;
int Global::Enable_Aging = 0;
int Global::Enable_Births = 0;
int Global::Enable_Deaths = 0;
int Global::Enable_Mobility = 0;
int Global::Enable_Migration = 0;
int Global::Track_infection_events = 0;
int Global::Track_age_distribution = 0;
int Global::Track_household_distribution = 0;
int Global::Verbose = 0;
int Global::Debug = 0;
int Global::Test = 0;
int Global::Days = 0;
int Global::Reseed_day = 0;
unsigned long Global::Seed = 0;
int Global::Epidemic_offset = 0;
int Global::Vaccine_offset = 0;
char Global::Start_date[256];

// global singleton objects
Population Global::Pop;
Place_List Global::Places;
Grid *Global::Cells = NULL;
Large_grid *Global::Large_cells;
Small_grid *Global::Small_cells;
Date *Global::Sim_Date = NULL;
Evolution *Global::Evol = NULL;

// global file pointers
FILE *Global::Statusfp = NULL;
FILE *Global::Outfp = NULL;
FILE *Global::Tracefp = NULL;
FILE *Global::Infectionfp = NULL;
FILE *Global::VaccineTracefp = NULL;
FILE *Global::Birthfp = NULL;
FILE *Global::Deathfp = NULL;
FILE *Global::Prevfp = NULL;
FILE *Global::Incfp = NULL;
FILE *Global::ErrorLogfp = NULL;

void Global::get_global_parameters() {
  get_param((char *) "verbose", &Global::Verbose);
  get_param((char *) "debug", &Global::Debug);
  get_param((char *) "test", &Global::Test);
  get_param((char *) "quality_control", &Global::Quality_control);
  get_param((char *) "days", &Global::Days);
  get_param((char *) "seed", &Global::Seed);
  get_param((char *) "epidemic_offset", &Global::Epidemic_offset);
  get_param((char *) "vaccine_offset", &Global::Vaccine_offset);
  get_param((char *) "start_date", Global::Start_date);
  get_param((char *) "reseed_day", &Global::Reseed_day);
  get_param((char *) "popdir", Global::Population_directory);
  get_param((char *) "outdir", Global::Output_directory);
  get_param((char *) "tracefile", Global::Tracefilebase);
  get_param((char *) "track_infection_events", &Global::Track_infection_events);
  get_param((char *) "track_age_distribution", &Global::Track_age_distribution);
  get_param((char *) "track_household_distribution", &Global::Track_household_distribution);
  get_param((char *) "vaccine_tracefile", Global::VaccineTracefilebase);
  get_param((char *) "incremental_trace", &Global::Incremental_Trace);
  get_param((char *) "trace_headers", &Global::Trace_Headers);
  get_param((char *) "behavior_model", Global::Behavior_model_type);
  get_param((char *) "diseases", &Global::Diseases);
  get_param((char *) "prob_accept_vaccine", &Global::Prob_accept_vaccine);
  get_param((char *) "prob_accept_another_vaccine_dose", &Global::Prob_accept_vaccine_dose);
  get_param((char *) "enable_large_grid", &Global::Enable_Large_grid);
  get_param((char *) "enable_small_grid", &Global::Enable_Small_grid);
  get_param((char *) "enable_aging", &Global::Enable_Aging);
  get_param((char *) "enable_births", &Global::Enable_Births);
  get_param((char *) "enable_deaths", &Global::Enable_Deaths);
  get_param((char *) "enable_mobility",&Global::Enable_Mobility);
  get_param((char *) "enable_migration",&Global::Enable_Migration);
  get_param((char *) "prevalencefile", Global::Prevfilebase);
  get_param((char *) "incidencefile", Global::Incfilebase);
}

