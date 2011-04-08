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
int Days;
unsigned long Seed;
int Epidemic_offset;
int Vaccine_offset;
char Start_date[256];
int Reseed_day;
char Population_directory[256];
char Output_directory[256];
char Tracefilebase[256];
char VaccineTracefilebase[256];
int All_diseases_antigenically_identical;
int Incremental_Trace;
int Trace_Headers;
int Random_start_day;
char Behavior_model_type[256];
int Quality_control;
int Diseases;
int Enable_Aging;
int Enable_Births;
int Enable_Deaths;
double Prob_accept_vaccine; // For Random_Congition_Model, need to find a way not to be global
double Prob_accept_vaccine_dose; //  For Random_Congition_Model, need to find a way not to be global

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
  get_param((char *) "days", &Days);
  get_param((char *) "seed", &Seed);
  get_param((char *) "epidemic_offset", &Epidemic_offset);
  get_param((char *) "vaccine_offset", &Vaccine_offset);
  get_param((char *) "start_date", Start_date);
  get_param((char *) "reseed_day", &Reseed_day);
  get_param((char *) "popdir", Population_directory);
  get_param((char *) "outdir", Output_directory);
  get_param((char *) "tracefile", Tracefilebase);
  get_param((char *) "vaccine_tracefile", VaccineTracefilebase);
  get_param((char *) "all_diseases_antigenically_identical", &All_diseases_antigenically_identical);
  get_param((char *) "incremental_trace", &Incremental_Trace);
  get_param((char *) "trace_headers", &Trace_Headers);
  get_param((char *) "behavior_model", Behavior_model_type);
  get_param((char *) "diseases", &Diseases);
  get_param((char *) "prob_accept_vaccine", &Prob_accept_vaccine);
  get_param((char *) "prob_accept_another_vaccine_dose",&Prob_accept_vaccine_dose);
  get_param((char *) "enable_aging",&Enable_Aging);
  get_param((char *) "enable_births",&Enable_Births);
  get_param((char *) "enable_deaths",&Enable_Deaths);
}
