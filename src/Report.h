/*
  This file is part of the pFRED system.

  Copyright (c) 2016, Carnegie Mellon University, Pittsburgh Supercomputing Center
  Jay DePasse

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Report.h
//

#ifndef _FRED_REPORT_H
#define _FRED_REPORT_H

#include <new>
#include <vector>
#include <deque>
#include <map>
#include <iostream>

#include "Random.h"
#include "Global.h"
#include "State.h"
#include "json.h"

using nlohmann::json;

struct Report_State {

  fred::Spin_Mutex mutex;
  std::vector< json > report_vector;

  void clear() {
    fred::Spin_Lock lock( mutex );
    report_vector.clear();
  }

  size_t report_vector_size() {
    fred::Spin_Lock lock( mutex );
    return report_vector.size();
  }

  void append(json & j) {
    fred::Spin_Lock lock( mutex );
    report_vector.push_back(j);
  }

  std::vector< json > & get_report_vector() {
    fred::Spin_Lock lock( mutex );
    return report_vector;
  }

  void reset() {
    fred::Spin_Lock lock( mutex );
    if ( report_vector.size() > 0 ) {
      report_vector = std::vector< json >();
    }
  }

  void print() {
    fred::Spin_Lock lock( mutex );
    std::vector< json >::iterator itr;
    for (itr = report_vector.begin(); itr < report_vector.end(); ++itr) {
      std::cout << *itr << std::endl;
    }
  }

};

class Report {

public:
  
  void setup();
  void print();
  void clear();
  void append(json & j);

protected:

  State< Report_State > report_state;

};


#endif // _FRED_REPORT_H
