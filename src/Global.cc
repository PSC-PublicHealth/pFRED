//
//
// File: Global.cc
//

#include "Global.h"
#include "Params.h"
#include "Population.h"
#include "Place_List.h"
#include "Grid.h"
#include "Large_Grid.h"
#include "Small_Grid.h"
#include "Seasonality.h"

// global runtime parameters
char Global::Population_directory[256];
char Global::Output_directory[256];
char Global::Tracefilebase[256];
char Global::VaccineTracefilebase[256];
int Global::Incremental_Trace = 0;
int Global::Trace_Headers = 0;
int Global::Random_start_day = 0;
int Global::Quality_control = 0;
int Global::RR_delay = 0;
int Global::Diseases = 0;
int Global::StrainEvolution = 0;
char Global::Prevfilebase[256];
char Global::Incfilebase[256];
char Global::ErrorLogbase[256];
int Global::Enable_Large_Grid = 0;
int Global::Enable_Small_Grid = 0;
int Global::Enable_Aging = 0;
int Global::Enable_Births = 0;
int Global::Enable_Deaths = 0;
int Global::Enable_Mobility = 0;
int Global::Enable_Migration = 0;
int Global::Enable_Travel = 0;
int Global::Track_infection_events = 0;
int Global::Track_age_distribution = 0;
int Global::Track_household_distribution = 0;
int Global::Track_network_stats = 0;
int Global::Verbose = 0;
int Global::Debug = 0;
int Global::Test = 0;
int Global::Days = 0;
int Global::Reseed_day = 0;
unsigned long Global::Seed = 0;
int Global::Epidemic_offset = 0;
int Global::Vaccine_offset = 0;
char Global::Start_date[256];
int Global::Enable_Seasonality = 0;
int Global::Enable_Climate = 0;
char Global::Seasonality_Timestep[256];

// global singleton objects
Population Global::Pop;
Place_List Global::Places;
Grid *Global::Cells = NULL;
Large_Grid *Global::Large_Cells;
Small_Grid *Global::Small_Cells;
Date *Global::Sim_Date = NULL;
Evolution *Global::Evol = NULL;
Seasonality * Global::Clim = NULL;

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
  Params::get_param((char *) "verbose", &Global::Verbose);
  Params::get_param((char *) "debug", &Global::Debug);
  Params::get_param((char *) "test", &Global::Test);
  Params::get_param((char *) "quality_control", &Global::Quality_control);
  Params::get_param((char *) "rr_delay", &Global::RR_delay);
  Params::get_param((char *) "days", &Global::Days);
  Params::get_param((char *) "seed", &Global::Seed);
  Params::get_param((char *) "epidemic_offset", &Global::Epidemic_offset);
  Params::get_param((char *) "vaccine_offset", &Global::Vaccine_offset);
  Params::get_param((char *) "start_date", Global::Start_date);
  Params::get_param((char *) "reseed_day", &Global::Reseed_day);
  Params::get_param((char *) "popdir", Global::Population_directory);
  Params::get_param((char *) "outdir", Global::Output_directory);
  Params::get_param((char *) "tracefile", Global::Tracefilebase);
  Params::get_param((char *) "track_infection_events", &Global::Track_infection_events);
  Params::get_param((char *) "track_age_distribution", &Global::Track_age_distribution);
  Params::get_param((char *) "track_network_stats", &Global::Track_network_stats);
  Params::get_param((char *) "track_household_distribution", &Global::Track_household_distribution);
  Params::get_param((char *) "vaccine_tracefile", Global::VaccineTracefilebase);
  Params::get_param((char *) "incremental_trace", &Global::Incremental_Trace);
  Params::get_param((char *) "trace_headers", &Global::Trace_Headers);
  Params:: get_param((char *) "diseases", &Global::Diseases);
  Params::get_param((char *) "enable_large_grid", &Global::Enable_Large_Grid);
  Params::get_param((char *) "enable_small_grid", &Global::Enable_Small_Grid);
  Params::get_param((char *) "enable_aging", &Global::Enable_Aging);
  Params::get_param((char *) "enable_births", &Global::Enable_Births);
  Params::get_param((char *) "enable_deaths", &Global::Enable_Deaths);
  Params::get_param((char *) "enable_mobility",&Global::Enable_Mobility);
  Params::get_param((char *) "enable_migration",&Global::Enable_Migration);
  Params::get_param((char *) "enable_travel",&Global::Enable_Travel);
  Params::get_param((char *) "prevalencefile", Global::Prevfilebase);
  Params::get_param((char *) "incidencefile", Global::Incfilebase);
  Params::get_param((char *) "enable_seasonality", &Global::Enable_Seasonality);
  Params::get_param((char *) "enable_climate", &Global::Enable_Climate);
  Params::get_param((char *) "seasonality_timestep_file", Global::Seasonality_Timestep);
}

