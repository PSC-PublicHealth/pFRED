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
int Start_day;
char Outfilebase[80];
char Tracefilebase[80];

// global file pointers
FILE *Statusfp;
FILE *Outfp;
FILE *Tracefp;

// global singleton objects
Population Pop;
Locations Loc;
int Strains;

void get_global_parameters() {
  get_param((char *) "verbose", &Verbose);
  get_param((char *) "debug", &Debug);
  get_param((char *) "test", &Test);
  get_param((char *) "runs", &Runs);
  get_param((char *) "days", &Days);
  get_param((char *) "seed", &Seed);
  get_param((char *) "start_day", &Start_day);
  get_param((char *) "outfile", Outfilebase);
  get_param((char *) "tracefile", Tracefilebase);
}

