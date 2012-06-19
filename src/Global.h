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
#include <bitset>

// for unit testing, use the line in Makefile: gcc -DUNITTEST ...
#ifdef UNITTEST
#define UNIT_TEST_VIRTUAL virtual
#else
#define UNIT_TEST_VIRTUAL
#endif

class Population;
class Place_List;
class Grid;
class Large_Grid;
class Small_Grid;
class Date;
class Evolution;
class Seasonality;

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
    // MAX_NUM_DISEASES sets the size of stl::bitsets and static arrays used throughout FRED
    // to store disease-specific flags and pointers; set to the smallest possible value 
    // for optimal performance and memory usage
    static const int MAX_NUM_DISEASES = 4;

    // household relationship codes
    static const int UNKNOWN_RELATION = 0;
    static const int HOUSEHOLDER = 1;
    static const int SPOUSE = 2;
    static const int NATURAL_CHILD = 3;
    static const int ADOPTED_CHILD = 4;
    static const int STEP_CHILD = 5;
    static const int SIBLING = 6;
    static const int PARENT = 7;
    static const int GRANDCHILD = 8;
    static const int PARENT_IN_LAW = 9;
    static const int SON_DAUGHTER_IN_LAW = 10;
    static const int OTHER_RELATIVE = 11;
    static const int BROTHER_SISTER_IN_LAW = 12;
    static const int NEPHEW_NIECE = 13;
    static const int GRANDPARENT = 14;
    static const int UNCLE_AUNT = 15;
    static const int COUSIN = 16;
    static const int BOARDER = 17;
    static const int HOUSEHMATE = 18;
    static const int PARTNER = 19;
    static const int FOSTER_CHILD = 20;
    static const int NON_RELATIVE = 21;

    // global runtime parameters
    static char Population_directory[];
    static char Output_directory[];
    static char Tracefilebase[];
    static char VaccineTracefilebase[];
    static int Incremental_Trace;
    static int Trace_Headers;
    static int Rotate_start_date;
    static int Quality_control;
    static int RR_delay;
    static int Diseases;
    static int StrainEvolution;
    static char Prevfilebase[];
    static char Incfilebase[];
    static int Enable_Behaviors;
    static int Track_infection_events;
    static int Track_age_distribution;
    static int Track_household_distribution;
    static int Track_network_stats;
    static int Verbose;
    static int Debug;
    static int Test;
    static int Days;
    static int Reseed_day;
    static unsigned long Seed;
    static char Start_date[];
    static int Epidemic_offset;
    static int Vaccine_offset;
    static char Seasonality_Timestep[];
    static double Work_absenteeism;
    static double School_absenteeism;
    static int Seed_age_lower_bound;
    static int Seed_age_upper_bound;

    //Boolean flags
    static bool Enable_Large_Grid;
    static bool Enable_Small_Grid;
    static bool Enable_Aging;
    static bool Enable_Births;
    static bool Enable_Deaths;
    static bool Enable_Migration;
    static bool Enable_Mobility;
    static bool Enable_Travel;
    static bool Enable_Local_Workplace_Assignment;
    static bool Enable_Seasonality;
    static bool Enable_Climate;
    static bool Seed_by_age;
    static bool Report_Prevalence;
    static bool Report_Incidence;
    static bool Enable_Vaccination;
    static bool Enable_Antivirals;
    static bool Use_Mean_Latitude;
    static bool Print_Household_Locations;
    static bool Report_Age_Of_Infection;
    static bool Report_Place_Of_Infection;
    static bool Report_Presenteeism;

    // global singleton objects
    static Population Pop;
    static Place_List Places;
    static Grid *Cells;
    static Large_Grid *Large_Cells;
    static Small_Grid *Small_Cells;
    static Date *Sim_Start_Date;
    static Date *Sim_Current_Date;
    static Evolution *Evol;
    static Seasonality *Clim;

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
    static FILE *Householdfp;


    /**
     * Fills the static variables with values from the parameter file.
     */
    static void get_global_parameters();
};

/* 
 * Put fred's global typedefs in namespace fred.
 */

namespace fred {
  /* 
   * bitset big enough to store flags for MAX_NUM_DISEASES
   * Global::MAX_NUM_DISEASES should be equal to Global::Diseases
   * for efficiency. 
   * IMPORTANT NOTE TO THE PROGRAMMER: 'Diseases' may be less than
   * 'MAX_NUM_DISEASES' so care should be taken when performing operations
   * (such as any(), flip(), reset(), etc.) on a disease_bitset to avoid
   * unintended setting/resetting flags for non-existent diseases.
   *
   */
  typedef std::bitset<Global::MAX_NUM_DISEASES> disease_bitset;
}

#endif // _FRED_GLOBAL_H
