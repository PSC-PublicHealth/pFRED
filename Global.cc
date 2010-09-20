//
//
// File: Global.cc
//

#include "Global.h"
#include "Params.h"

// global runtime parameters
int Verbose;
int Debug;
int Test;
int Runs;
int Days;
unsigned long Seed;
int Start_day_of_week;
int Reseed_day;
char Output_directory[256];
char Tracefilebase[80];
char VaccineTracefilebase[80];
int All_strains_antigenically_identical;
int Incremental_Trace;
int Trace_Headers;
int Random_start_day;
char Cognitive_model_type[80];
int Quality_control;
int Strains;

// global file pointers
FILE *Statusfp;
FILE *Outfp;
FILE *Tracefp;
FILE *VaccineTracefp;

void get_global_parameters() {
  get_param((char *) "verbose", &Verbose);
  get_param((char *) "debug", &Debug);
  get_param((char *) "test", &Test);
  get_param((char *) "quality_control", &Quality_control);
  get_param((char *) "runs", &Runs);
  get_param((char *) "days", &Days);
  get_param((char *) "seed", &Seed);
  get_param((char *) "start_day_of_week", &Start_day_of_week);
  get_param((char *) "reseed_day", &Reseed_day);
  get_param((char *) "outdir", Output_directory);
  get_param((char *) "tracefile", Tracefilebase);
  get_param((char *) "vaccine_tracefile", VaccineTracefilebase);
  get_param((char *) "all_strains_antigenically_identical", &All_strains_antigenically_identical);
  get_param((char *) "incremental_trace", &Incremental_Trace);
  get_param((char *) "trace_headers", &Trace_Headers);
  get_param((char *) "cognitive_model", Cognitive_model_type);
  get_param((char *) "strains", &Strains);
}
