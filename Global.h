/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */
//
//
// File: Global.h
//

#ifndef _FRED_GLOBAL_H
#define _FRED_GLOBAL_H

#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>

// global constants
#define DAYS_PER_WEEK 7

// global runtime parameters
extern int Verbose;
extern int Debug;
extern int Test;
extern int Runs;
extern int Days;
extern int Reseed_day;
extern unsigned long Seed;
extern int Start_day;
extern char Outfilebase[80];
extern char Tracefilebase[80];
extern int All_strains_antigenically_identical;
extern int SEiIR_model;
extern char VaccineTracefilebase[80];
extern int Show_cases;
extern int Incremental_Trace;
extern int Trace_Headers;
extern int Random_start_day;
extern int Random_location_order;
extern char Cognitive_model_type[80];

// global file pointers
extern FILE *Statusfp;
extern FILE *Outfp;
extern FILE *Tracefp;
extern FILE *VaccineTracefp;

// global singleton objects
#include "Population.h"
// class Population;
extern Population Pop;

#include "Locations.h"
// class Locations;
extern Locations Loc;

void get_global_parameters();

#endif // _FRED_GLOBAL_H
