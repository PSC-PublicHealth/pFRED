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
#define ADULT_AGE 18
#define SCHOOL_AGE 5
#define RETIREMENT_AGE 67

// global runtime parameters
extern int Verbose;
extern int Debug;
extern int Test;
extern int Days;
extern int Reseed_day;
extern unsigned long Seed;
extern int Epidemic_offset;
extern int Vaccine_offset;
extern char Start_date[];

extern char Population_directory[];
extern char Output_directory[];
extern char Tracefilebase[];
extern char Eventfilebase[];
extern char VaccineTracefilebase[];
extern int All_diseases_antigenically_identical;
extern int Incremental_Trace;
extern int Trace_Headers;
extern int Random_start_day;
extern char Behavior_model_type[];
extern int Quality_control;
extern int Diseases;
extern double Prob_accept_vaccine;
extern double Prob_accept_vaccine_dose;
extern int Enable_Aging;
extern int Enable_Births;
extern int Enable_Deaths;
extern int StrainEvolution;
extern char Prevfilebase[256];
extern char Incfilebase[256];
extern int Track_infection_events;

// global file pointers
extern FILE *Statusfp;
extern FILE *Outfp;
extern FILE *Tracefp;
extern FILE *Infectionfp;
extern FILE *VaccineTracefp;
extern FILE *Prevfp;
extern FILE *Incfp;
extern FILE *Birthfp;
extern FILE *Deathfp;

// global singleton objects
class Population;
extern Population Pop;

class Place_List;
extern Place_List Places;

class Grid;
extern Grid Environment;

class Date;
extern Date * Sim_Date;

class Evolution;
extern Evolution *Evol;

void get_global_parameters();

#endif // _FRED_GLOBAL_H
