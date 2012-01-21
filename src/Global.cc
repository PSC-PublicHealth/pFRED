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
int Global::Enable_Behaviors = 0;
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
char Global::Seasonality_Timestep[256];
double Global::Work_absenteeism = 0.0;
double Global::School_absenteeism = 0.0;
bool Global::Enable_Large_Grid = false;
bool Global::Enable_Small_Grid = false;
bool Global::Enable_Aging = false;
bool Global::Enable_Births = false;
bool Global::Enable_Deaths = false;
bool Global::Enable_Mobility = false;
bool Global::Enable_Migration = false;
bool Global::Enable_Travel = false;
bool Global::Enable_Local_Workplace_Assignment = false;
bool Global::Enable_Seasonality = false;
bool Global::Enable_Climate = false;

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

  Params::get_param_from_string("verbose", &Global::Verbose);
  Params::get_param_from_string("debug", &Global::Debug);
  Params::get_param_from_string("test", &Global::Test);
  Params::get_param_from_string("quality_control", &Global::Quality_control);
  Params::get_param_from_string("rr_delay", &Global::RR_delay);
  Params::get_param_from_string("days", &Global::Days);
  Params::get_param_from_string("seed", &Global::Seed);
  Params::get_param_from_string("epidemic_offset", &Global::Epidemic_offset);
  Params::get_param_from_string("vaccine_offset", &Global::Vaccine_offset);
  Params::get_param_from_string("start_date", Global::Start_date);
  Params::get_param_from_string("reseed_day", &Global::Reseed_day);
  Params::get_param_from_string("popdir", Global::Population_directory);
  Params::get_param_from_string("outdir", Global::Output_directory);
  Params::get_param_from_string("tracefile", Global::Tracefilebase);
  Params::get_param_from_string("track_infection_events", &Global::Track_infection_events);
  Params::get_param_from_string("track_age_distribution", &Global::Track_age_distribution);
  Params::get_param_from_string("track_network_stats", &Global::Track_network_stats);
  Params::get_param_from_string("track_household_distribution", &Global::Track_household_distribution);
  Params::get_param_from_string("vaccine_tracefile", Global::VaccineTracefilebase);
  Params::get_param_from_string("incremental_trace", &Global::Incremental_Trace);
  Params::get_param_from_string("trace_headers", &Global::Trace_Headers);
  Params::get_param_from_string("diseases", &Global::Diseases);
  Params::get_param_from_string("enable_behaviors",&Global::Enable_Behaviors);
  Params::get_param_from_string("prevalencefile", Global::Prevfilebase);
  Params::get_param_from_string("incidencefile", Global::Incfilebase);
  Params::get_param_from_string("seasonality_timestep_file", Global::Seasonality_Timestep);
  Params::get_param_from_string("work_absenteeism", &Global::Work_absenteeism);
  Params::get_param_from_string("school_absenteeism", &Global::School_absenteeism);

  //Set all of the boolean flags
  int temp_int = 0;
  Params::get_param_from_string("enable_large_grid", &temp_int);
  Global::Enable_Large_Grid = temp_int;
  Params::get_param_from_string("enable_small_grid", &temp_int);
  Global::Enable_Small_Grid = temp_int;
  Params::get_param_from_string("enable_aging", &temp_int);
  Global::Enable_Aging = temp_int;
  Params::get_param_from_string("enable_births", &temp_int);
  Global::Enable_Births = temp_int;
  Params::get_param_from_string("enable_deaths", &temp_int);
  Global::Enable_Deaths = temp_int;
  Params::get_param_from_string("enable_mobility",&temp_int);
  Global::Enable_Mobility = temp_int;
  Params::get_param_from_string("enable_migration",&temp_int);
  Global::Enable_Migration = temp_int;
  Params::get_param_from_string("enable_travel",&temp_int);
  Global::Enable_Travel = temp_int;
  Params::get_param_from_string("enable_local_workplace_assignment",&temp_int);
  Global::Enable_Local_Workplace_Assignment = temp_int;
  Params::get_param_from_string("enable_seasonality", &temp_int);
  Global::Enable_Seasonality = temp_int;
  Params::get_param_from_string("enable_climate", &temp_int);
  Global::Enable_Climate = temp_int;
}

