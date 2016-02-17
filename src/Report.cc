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
// File: Report.cc
//

#include "Report.h"
#include "Global.h"
#include "Params.h"
#include "Utils.h"

void Report::setup() {

  // Following arithmetic estimates the optimal number of thread-safe states
  // Should always be 1 <= dim <= max_num_threads.  Each state is thread-safe,
  // but increasing the number of states reduces lock-contention.  Minimizing
  // the number of states saves memory.  The values used here must be determined
  // through experimentation.  Optimal values will vary based on the particular
  // cpu, number of threads, calibrated attack rate, population density, etc...
  unsigned int dim = fred::omp_get_max_threads();
  dim = dim == 0 ? 1 : dim;
  dim = dim <= fred::omp_get_max_threads() ? dim : fred::omp_get_max_threads();
  // Initialize specified number of states
  report_state = State< Report_State >( dim );
}

void Report::print() {
  for (int i = 0; i < report_state.size(); ++i) {
    report_state(i).print(); 
  }
}

void Report::clear() {
  for (int i = 0; i < report_state.size(); ++i) {
    report_state(i).clear(); 
  }
}

void Report::append(json & j) {
  report_state().append(j); 
}
