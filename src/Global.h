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

class Population;
class Place_List;
class Grid;
class Large_grid;
class Small_grid;
class Date;
class Evolution;

/**
 * This class contains the static variables used by the FRED program.  The variables all have public access,
 * so they can be modified by any class that uses the <code>Global</code> class.  However, by making them
 * static class variables, the compiler forces the programmer to reference them using the full nomenclature
 * <code>Global::variable_name</code>, which in turn makes it clear for code maintenance where the actual
 * variable resides.
 *
 * The static method <code>get_global_parameters</code> is used to set the parameters from the parameter file.
 */
class Global {
  public:
    // global constants
    static const int DAYS_PER_WEEK = 7;
    static const int ADULT_AGE = 18;
    static const int SCHOOL_AGE = 5;
    static const int RETIREMENT_AGE = 67;

    // global runtime parameters
    static char Population_directory[];
    static char Output_directory[];
    static char Tracefilebase[];
    static char Eventfilebase[];
    static char VaccineTracefilebase[];
    static int Incremental_Trace;
    static int Trace_Headers;
    static int Random_start_day;
    static char Behavior_model_type[];
    static int Quality_control;
    static int Diseases;
    static double Prob_accept_vaccine;
    static double Prob_accept_vaccine_dose;
    static int StrainEvolution;
    static char Prevfilebase[];
    static char Incfilebase[];
    static char ErrorLogbase[];
    static int Enable_Large_grid;
    static int Enable_Small_grid;
    static int Enable_Aging;
    static int Enable_Births;
    static int Enable_Deaths;
    static int Enable_Migration;
    static int Enable_Mobility;
    static int Track_infection_events;
    static int Track_age_distribution;
    static int Track_household_distribution;
    static int Verbose;
    static int Debug;
    static int Test;
    static int Days;
    static int Reseed_day;
    static unsigned long Seed;
    static int Epidemic_offset;
    static int Vaccine_offset;
    static char Start_date[];

    // global singleton objects
    static Population Pop;
    static Place_List Places;
    static Grid *Cells;
    static Large_grid *Large_cells;
    static Small_grid *Small_cells;
    static Date *Sim_Date;
    static Evolution *Evol;

    // global file pointers
    static FILE *Statusfp;
    static FILE *Outfp;
    static FILE *Tracefp;
    static FILE *Infectionfp;
    static FILE *VaccineTracefp;
    static FILE *Birthfp;
    static FILE *Deathfp;
    static FILE *Prevfp;
    static FILE *Incfp;
    static FILE *ErrorLogfp;

    /**
     * Fills the static variables with values from the parameter file.
     */
    static void get_global_parameters();
};

#endif // _FRED_GLOBAL_H
