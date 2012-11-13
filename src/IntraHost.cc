/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/



#include "IntraHost.h"
#include "DefaultIntraHost.h"
#include "FixedIntraHost.h"
#include "Utils.h"
//#include "ODEIntraHost.h"

using namespace std;

void IntraHost :: setup(Disease *disease) {
  this->disease = disease;
  max_days = 0;
  }

IntraHost *IntraHost :: newIntraHost(int type) {
  switch(type) {
    case 0:
      return new DefaultIntraHost;

    case 1:
      return new FixedIntraHost;

      //    case 2:
      //      return new ODEIntraHost;

    default:
      FRED_WARNING("Unknown IntraHost type (%d) supplied to IntraHost factory.  Using DefaultIntraHost.\n", type);
      return new DefaultIntraHost;
    }
  }

int IntraHost :: get_days_symp() {
  return 0;
  }
