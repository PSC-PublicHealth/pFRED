/*
   This file is part of the FRED system.

   Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
   Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
   Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

   Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
   more information.
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
static char ErrorFilename[FRED_STRING_SIZE];

void Utils::fred_abort(const char* format, ...){

  // open ErrorLog file if it doesn't exist
  if(Global::ErrorLogfp == NULL){
    Global::ErrorLogfp = fopen(ErrorFilename, "w");
    if (Global::ErrorLogfp == NULL) {
      // output to stdout
      printf("FRED ERROR: Can't open errorfile %s\n", ErrorFilename);
      // current error message:
      va_list ap;
      va_start(ap,format);
      printf("FRED ERROR: ");
      vprintf(format,ap);
      va_end(ap);
      fflush(stdout);
      fred_end();
      abort();
    }
  }

  // output to error file
  va_list ap;
  va_start(ap,format);
  fprintf(Global::ErrorLogfp,"FRED ERROR: ");
  vfprintf(Global::ErrorLogfp,format,ap);
  va_end(ap);
  fflush(Global::ErrorLogfp);

  // output to stdout
  va_start(ap,format);
  printf("FRED ERROR: ");
  vprintf(format,ap);
  va_end(ap);
  fflush(stdout);

  fred_end();
  abort();
}

void Utils::fred_warning(const char* format, ...){

  // open ErrorLog file if it doesn't exist
  if(Global::ErrorLogfp == NULL){
    Global::ErrorLogfp = fopen(ErrorFilename, "w");
    if (Global::ErrorLogfp == NULL) {
      // output to stdout
      printf("FRED ERROR: Can't open errorfile %s\n", ErrorFilename);
      // current error message:
      va_list ap;
      va_start(ap,format);
      printf("FRED WARNING: ");
      vprintf(format,ap);
      va_end(ap);
      fflush(stdout);
      fred_end();
      abort();
    }
  }

  // output to error file
  va_list ap;
  va_start(ap,format);
  fprintf(Global::ErrorLogfp,"FRED WARNING: ");
  vfprintf(Global::ErrorLogfp,format,ap);
  va_end(ap);
  fflush(Global::ErrorLogfp);

  // output to stdout
  va_start(ap,format);
  printf("FRED WARNING: ");
  vprintf(format,ap);
  va_end(ap);
  fflush(stdout);
}

void Utils::fred_open_output_files(char * directory, int run){
  char filename[FRED_STRING_SIZE];

  // ErrorLog file is created at the first warning or error
  Global::ErrorLogfp = NULL;
  sprintf(ErrorFilename, "%s/err%d.txt", directory, run);

  sprintf(filename, "%s/out%d.txt", directory, run);
  Global::Outfp = fopen(filename, "w");
  if (Global::Outfp == NULL) {
    Utils::fred_abort("Can't open %s\n", filename);
  }
  Global::Tracefp = NULL;
  if (strcmp(Global::Tracefilebase, "none") != 0) {
    sprintf(filename, "%s/trace%d.txt", directory, run);
    Global::Tracefp = fopen(filename, "w");
    if (Global::Tracefp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Infectionfp = NULL;
  if (Global::Track_infection_events) {
    sprintf(filename, "%s/infections%d.txt", directory, run);
    Global::Infectionfp = fopen(filename, "w");
    if (Global::Infectionfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::VaccineTracefp = NULL;
  if (strcmp(Global::VaccineTracefilebase, "none") != 0) {
    sprintf(filename, "%s/vacctr%d.txt", directory, run);
    Global::VaccineTracefp = fopen(filename, "w");
    if (Global::VaccineTracefp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  if (Global::Track_vaccine_infection_events) {
      Global::VaccineInfectionTrackerfp = NULL;
      if (strcmp(Global::VaccineInfectionTrackerfilebase, "none") != 0) {
          sprintf(filename, "%s/%s%d.txt", directory, Global::VaccineInfectionTrackerfilebase, run);
      }
      else {
          sprintf(filename, "%s/vaccinf%d.txt", directory, run);
      }
      Global::VaccineInfectionTrackerfp = fopen(filename, "w");
      if (Global::VaccineInfectionTrackerfp == NULL) {
        Utils::fred_abort("Can't open %s\n", filename);
      }
  }
  Global::Birthfp = NULL;
  if (Global::Enable_Births) {
    sprintf(filename, "%s/births%d.txt", directory, run);
    Global::Birthfp = fopen(filename, "w");
    if (Global::Birthfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Deathfp = NULL;
  if (Global::Enable_Deaths) {
    sprintf(filename, "%s/deaths%d.txt", directory, run);
    Global::Deathfp = fopen(filename, "w");
    if (Global::Deathfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }
  Global::Immunityfp = NULL;
  if (strcmp(Global::Immunityfilebase, "none") != 0) {
    sprintf(filename, "%s/immunity%d.txt", directory, run);
    Global::Immunityfp = fopen(filename, "w");
    if (Global::Immunityfp == NULL) {
      Utils::fred_abort("Help! Can't open %s\n", filename);
    }
    Global::Report_Immunity = true;
  }
  Global::Householdfp = NULL;
  if (Global::Print_Household_Locations) {
    sprintf(filename, "%s/households.txt", directory);
    Global::Householdfp = fopen(filename, "w");
    if (Global::Householdfp == NULL) {
      Utils::fred_abort("Can't open %s\n", filename);
    }
  }

  return;
}

void Utils::fred_make_directory(char * directory) {
  mode_t mask;        // the user's current umask
  mode_t mode = 0777; // as a start
  mask = umask(0); // get the current mask, which reads and sets...
  umask(mask);     // so now we have to put it back
  mode ^= mask;    // apply the user's existing umask
  if (0!=mkdir(directory, mode) && EEXIST!=errno) // make it
    Utils::fred_abort("mkdir(%s) failed with %d\n", directory, errno); // or die
}


void Utils::fred_end(void){
  // This is a function that cleans up FRED and exits
  if (Global::Outfp != NULL) fclose(Global::Outfp);
  if (Global::Tracefp != NULL) fclose(Global::Tracefp);
  if (Global::Infectionfp != NULL) fclose(Global::Infectionfp);
  if (Global::VaccineTracefp != NULL) fclose(Global::VaccineTracefp);
  if (Global::Prevfp != NULL) fclose(Global::Prevfp);
  if (Global::Incfp != NULL) fclose(Global::Incfp);
  if (Global::Immunityfp != NULL) fclose(Global::Immunityfp);
  if (Global::Householdfp != NULL) fclose(Global::Householdfp);
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

void Utils::fred_start_timer( time_t * lap_start_time ) {
  time( lap_start_time );
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

void Utils::fred_print_lap_time( time_t * start_lap_time, const char* format, ...) {
  time_t stop_lap_time;
  time( &stop_lap_time );
  va_list ap;
  va_start(ap,format);
  vfprintf(Global::Statusfp,format,ap);
  va_end(ap);
  fprintf(Global::Statusfp, " took %d seconds\n",
      (int) ( stop_lap_time - ( *start_lap_time ) ) );
  fflush(Global::Statusfp);
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

void Utils::fred_verbose_statusfp(int verbosity, const char* format, ...) {
  if (Global::Verbose > verbosity) {
    va_list ap;
    va_start(ap,format);
    vfprintf(Global::Statusfp,format,ap);
    va_end(ap);
    fflush(Global::Statusfp);
  }
}

void Utils::fred_log(const char* format, ...){
  va_list ap;
  va_start(ap,format);
  vfprintf(Global::Statusfp,format,ap);
  va_end(ap);
  fflush(Global::Statusfp);
}

void Utils::fred_report(const char* format, ...){
  va_list ap;
  va_start(ap,format);
  vfprintf(Global::Outfp,format,ap);
  fflush(Global::Outfp);
  va_start(ap,format);
  vfprintf(Global::Statusfp,format,ap);
  fflush(Global::Statusfp);
  va_end(ap);
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

/********************************************************
 * Input: in_str is a csv string, possiblly ending with \n
 * Output out_str is a csv string with empty fields replaced
 * by replacement string.
 *
 * Notes: It is assumed that empty strings do not have white space
 * between commas. Newlines in the input string are ignored, so the
 * output string contains no newlines.
 *        
 */
void Utils::replace_csv_missing_data(char* out_str, char* in_str, const char* replacement) {
  // printf("in = |%s|replacement = %s\n",in_str,replacement);
  int i = 0;
  int j = 0;
  int new_field = 1;
  while (in_str[i] != '\0') {
    if (in_str[i] == '\n') {
      i++;
    }
    else if (new_field && in_str[i] == ',') {
      // field is missing, so insert replacement
      int k = 0;
      while (replacement[k] != '\0') { out_str[j++] = replacement[k++]; }
      out_str[j++] = ',';
      i++;
      new_field = 1;
    }
    else if (in_str[i] == ',') {
      out_str[j++] = in_str[i++];
      new_field = 1;
    }
    else {
      out_str[j++] = in_str[i++];
      new_field = 0;
    }
  }
  // printf("new_field = %d\n", new_field); fflush(stdout);
  if (new_field) {
    // last field is missing
    int k = 0;
    while (replacement[k] != '\0') { out_str[j++] = replacement[k++]; }
  }
  out_str[j] = '\0';
  // printf("out = |%s|\n",out_str); fflush(stdout);
}


void Utils::get_next_token(char * out_string, char ** input_string) {
  char *token;
  token = strsep(input_string,",");

  // if the field is empty, we report a value of "-1"
  if (*token == '\0') {
    strcpy(out_string,"-1");
    return;
  }

  // token is non-empty
  strcpy(out_string,token);

  // if the token contains an opening quote but not closing quote, then
  // it was truncated by an intervening comma, so we have to retrieve
  // the remainder of the field:
  if (*token == '"') {
    char * c;
    c = token;
    c++;					// skip opening quote
    while ((*c != '"') && (*c != '\0')) c++;    // search for closing quote
    if (*c == '\0') {				// no closing quote
      char *remainder;
      remainder = strsep(input_string, "\"");
      // concatenate remainder of field onto out_string
      (void) strncat(out_string, remainder, sizeof(out_string) - strlen(remainder) - 1);
      // add closing quote
      (void) strcat(out_string, "\"");
      // retrieve rest of field up to next comma (and verify that this is empty)
      remainder = strsep(input_string, ",");
      assert(*remainder == '\0');
    }
  }
  return;
}

// remove non-NULL char c from string s with length at most maxlen
// string must be null-terminated.
void Utils::delete_char(char *s, char c, int maxlen) {
  int len = std::strlen( s );

  // abort if greater than or equal to maximum length 
  if (len >= maxlen)
    return;

  // do not remove NULL characters
  if ( c == '\0' )
    return;

  char *new_s = s;
  while (*s != '\0') {
    if ( *s != c )
      *new_s++ = *s;
    s++;
  }
  *new_s = '\0';
}

// replace multiple white spaces with a single space in string s
void Utils::normalize_white_space(char *s) {
  char *new_s = s;
  // printf("new_s = |%s|\n", new_s); fflush(stdout);
  int started = 0;
  char *token;
  while ((token = strsep(&s, " \t")) != NULL) {
    if (*token != '\0') {
      // printf("token = |%s|\n", token); fflush(stdout);
      char *t = token;
      if (started) *new_s++ = ' ';
      while (*t != '\0') *new_s++ = *t++;
      *new_s = '\0';
      started = 1;
      // printf("new_s = |%s|\n", new_s); fflush(stdout);
    }
  }
}

// TODO Utils::tokens class for split_by_delim
//      - stores tokens internally as vector of strings
//      - allows access to const char * with const operator[]
//      - can be passed as refence to split methods (treated like vector)

// splits a string by delimiter, loads into vector passed by reference
Utils::Tokens & Utils::split_by_delim( const std::string & str,
    const char delim, Tokens & tokens,
    bool collapse_consecutive_delims ) {

  std::stringstream ss( str );
  std::string item;

  std::string quoted_item;
  quoted_item.clear();

  while ( std::getline( ss, item, delim ) ) {
    if ( !item.empty() ) {
      if ( item.size() > 1 && ( item.at(0) == '\"' || item.at(0) == '\'' ) ) {
        quoted_item = item;
        continue;
      }
      else if ( !quoted_item.empty() ) {
        if ( item.at(item.size()-1) != quoted_item.at(0) ) {
          quoted_item = quoted_item + "," + item;
          continue;
        }
        else {
          item = quoted_item + "," + item;
          item.erase(0,1);
          item.erase(item.size()-1,1);
          quoted_item.clear();
        }
      }
    }
    if ( !item.empty() || !collapse_consecutive_delims ) {
      tokens.push_back( item );
    }
  }
  int trim_size = tokens.back().size();
  std::string::reverse_iterator rit = tokens.back().rbegin();
  for ( ; rit != tokens.back().rend(); ++rit ) {
    if ( (*rit) == '\n' || (*rit) == '\r' ) {
      --( trim_size );
    }
    else {
      if ( trim_size < tokens.back().size() ) {
        tokens.back().resize( trim_size );
      }
      break;
    }
  }
  return tokens;
}

// splits a string by delimiter, returns result in a new vector
Utils::Tokens Utils::split_by_delim( const std::string & str,
    const char delim, bool collapse_consecutive_delims ) {

  Tokens tokens;
  return split_by_delim( str, delim, tokens, collapse_consecutive_delims );
}

Utils::Tokens & Utils::split_by_delim( const char * str,
    const char delim, Tokens & tokens,
    bool collapse_consecutive_delims ) {

  return split_by_delim( std::string( str ), delim, tokens,
      collapse_consecutive_delims );
}

Utils::Tokens Utils::split_by_delim( const char * str,
    const char delim, bool collapse_consecutive_delims ) {

  return split_by_delim( std::string( str ), delim,
      collapse_consecutive_delims );
}


