/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Utils.h
//

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <stdarg.h>
#include <stdio.h>

using namespace std;

namespace Utils{
  void fred_abort(const char* format,...);
  void fred_warning(const char* format,...);
  void fred_open_output_files(char * directory, int run);
  void fred_end();
  void fred_print_wall_time(const char* format, ...);
  void fred_start_timer();
  void fred_start_day_timer();
  void fred_print_day_timer(int day);
  void fred_print_finish_timer();
  void fred_print_lap_time(const char* format, ...);
  void fred_verbose(int verbosity, const char* format, ...);
  void fred_log(const char* format, ...);
  void fred_report(const char* format, ...);
  FILE *fred_open_file(char * filename);
  void get_fred_file_name(char * filename);
  void fred_print_resource_usage(int day);
}

#endif /* UTILS_H_ */
