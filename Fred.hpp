/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Fred.h
//

#ifndef _FRED_H
#define _FRED_H

#include "Disease.hpp"
#include "Pop.hpp"
#include "Loc.hpp"
#include "Params.hpp"

void get_global_parameters();
void setup(char *paramfile);
void cleanup(int run);
void run_sim(int run);

#endif // _FRED_H
