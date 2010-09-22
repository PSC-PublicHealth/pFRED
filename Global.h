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
#include <assert.h>

// for unit testing, use the line in Makefile: gcc -DUNITTEST ...
#ifdef UNITTEST
#define UNIT_TEST_VIRTUAL virtual
#else
#define UNIT_TEST_VIRTUAL
#endif

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
extern int Start_day_of_week;
extern char Population_directory[];
extern char Output_directory[];
extern char Tracefilebase[];
extern char VaccineTracefilebase[];
extern int All_strains_antigenically_identical;
extern int Incremental_Trace;
extern int Trace_Headers;
extern int Random_start_day;
extern char Cognitive_model_type[];
extern int Quality_control;
extern int Strains;

// global file pointers
extern FILE *Statusfp;
extern FILE *Outfp;
extern FILE *Tracefp;
extern FILE *VaccineTracefp;

// global singleton objects
class Population;
extern Population Pop;

class Locations;
extern Locations Loc;

class Community;
extern Community * community;

void get_global_parameters();

#endif // _FRED_GLOBAL_H
