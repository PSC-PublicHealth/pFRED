/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/
//
//
// File: Utils.cc
//

#include "Utils.h"
#include "Global.h"
#include <stdlib.h>
#include <string.h>

using namespace std;
static time_t start_timer, stop_timer, fred_timer, day_timer;


void Utils::fred_abort(const char* format, ...){

  if(Global::ErrorLogfp != NULL){
    va_list ap;
    va_start(ap,format);
    fprintf(Global::ErrorLogfp,"FRED ERROR: ");
    vfprintf(Global::ErrorLogfp,format,ap);
    va_end(ap);
    fflush(Global::ErrorLogfp);
  }
  va_list ap;
  va_start(ap,format);
  printf("FRED ERROR: ");
  vprintf(format,ap);
  va_end(ap);
  fflush(stdout);

  fred_end();
  abort();
}

void Utils::fred_warning(const char* format, ...){

  if(Global::ErrorLogfp != NULL){
    va_list ap;
    va_start(ap,format);
    fprintf(Global::ErrorLogfp,"FRED WARNING: ");
    vfprintf(Global::ErrorLogfp,format,ap);
    va_end(ap);
    fflush(Global::ErrorLogfp);
  }
  va_list ap;
  va_start(ap,format);
  printf("FRED WARNING: ");
  vprintf(format,ap);
  va_end(ap);
  fflush(stdout);
}


void Utils::fred_end(void){
  // This is a function that cleans up FRED and exits
  if (Global::Outfp != NULL) fclose(Global::Outfp);
  if (Global::Tracefp != NULL) fclose(Global::Tracefp);
  if (Global::Infectionfp != NULL) fclose(Global::Infectionfp);
  if (Global::VaccineTracefp != NULL) fclose(Global::VaccineTracefp);
  if (Global::Prevfp != NULL) fclose(Global::Prevfp);
  if (Global::Incfp != NULL) fclose(Global::Incfp);
}


void Utils::fred_print_wall_time(const char* format, ...) {
  time_t clock;
  time(&clock);
  va_list ap;
  va_start(ap,format);
  vfprintf(Global::Statusfp,format,ap);
  va_end(ap);
  fprintf(Global::Statusfp," %s",ctime(&clock));
  fflush(Global::Statusfp);
}

void Utils::fred_start_timer() {
  time(&fred_timer);
  start_timer = fred_timer;
}

void Utils::fred_start_day_timer() {
  time(&day_timer);
  start_timer = day_timer;
}

void Utils::fred_print_day_timer(int day) {
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d took %d seconds\n",
	  day, (int) (stop_timer-day_timer));
  fflush(Global::Statusfp);
  start_timer = stop_timer;
}

void Utils::fred_print_finish_timer() {
  time(&stop_timer);
  fprintf(Global::Statusfp, "FRED took %d seconds\n",
	  (int)(stop_timer-fred_timer));
  fflush(Global::Statusfp);
}

void Utils::fred_print_lap_time(const char* format, ...) {
  time(&stop_timer);
  va_list ap;
  va_start(ap,format);
  vfprintf(Global::Statusfp,format,ap);
  va_end(ap);
  fprintf(Global::Statusfp, " took %d seconds\n",
	  (int) (stop_timer - start_timer));
  fflush(Global::Statusfp);
  start_timer = stop_timer;
}

void Utils::fred_verbose(int verbosity, const char* format, ...){
  if (Global::Verbose > verbosity) {
    va_list ap;
    va_start(ap,format);
    vprintf(format,ap);
    va_end(ap);
    fflush(stdout);
  }
}

FILE *Utils::fred_open_file(char * filename) {
  FILE *fp;
  get_fred_file_name(filename);
  printf("fred_open_file: opening file %s for reading\n", filename);
  fp = fopen(filename, "r");
  return fp;
}

void Utils::get_fred_file_name(char * filename) {
  string str;
  str.assign(filename);
  if (str.compare(0,10,"$FRED_HOME") == 0) {
    char * fred_home = getenv("FRED_HOME");
    if (fred_home != NULL) {
      str.erase(0,10);
      str.insert(0,fred_home);
      strcpy(filename, str.c_str());
    }
    else {
      fred_abort("get_fred_file_name: the FRED_HOME environmental variable cannot be found\n");
    }
  }
}

#include <sys/resource.h>
#define   RUSAGE_SELF     0
#define   RUSAGE_CHILDREN     -1
/*
struct rusage {
  struct timeval ru_utime; // user time used
  struct timeval ru_stime; // system time used
  long ru_maxrss;          // integral max resident set size
  long ru_ixrss;           // integral shared text memory size
  long ru_idrss;           // integral unshared data size
  long ru_isrss;           // integral unshared stack size
  long ru_minflt;          // page reclaims
  long ru_majflt;          // page faults
  long ru_nswap;           // swaps
  long ru_inblock;         // block input operations
  long ru_oublock;         // block output operations
  long ru_msgsnd;          // messages sent
  long ru_msgrcv;          // messages received
  long ru_nsignals;        // signals received
  long ru_nvcsw;           // voluntary context switches
  long ru_nivcsw;          // involuntary context switches
};
*/

void Utils::fred_print_resource_usage(int day) {
  rusage r_usage;
  getrusage(RUSAGE_SELF, &r_usage);
  printf("day %d maxrss %ld\n",
	 day, r_usage.ru_maxrss);
  fflush(stdout);
}
