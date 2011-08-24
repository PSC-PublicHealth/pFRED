/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.cc
//

#include "Strain.h"

#include <iostream>
#include <stdio.h>
#include <new>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

#include "Global.h"
#include "Params.h"
#include "Population.h"
#include "Random.h"
#include "Age_Map.h"
#include "Epidemic.h"
#include "Timestep_Map.h"
#include "Disease.h"

Strain::Strain() {
  transmissibility = -1.0;
  disease = NULL;
  strainData = NULL;
}

Strain::~Strain() {
  if(strainData != NULL) delete strainData;
}

void Strain::setup(int strain, Disease *disease, map<string, double> *data, double trans) {
  id = strain;
  this->disease = disease;
  strainData = data;
  transmissibility = trans;
}

void Strain::setup(int strain, Disease *disease) {
  map<string, double> *data = new map<string, double>;
  Params::get_double_indexed_param_map ("strain_data", disease->get_id(), strain, data);

  double trans;
  Params::get_double_indexed_param("transmissibility", disease->get_id(), strain, &trans);

  setup(strain, disease, data, trans);

  printf("Strain setup finished\n");
  fflush(stdout);

  if (Global::Verbose > 0) print();
}

void Strain::print() {
  printf("Strain %d trans %e\n", id, transmissibility);

  printf("data:  ");

  for(map<string, double>::iterator it = strainData->begin(); it != strainData->end(); it++) {
    printf("%s:%lf ", (it->first).c_str(), it->second);
  }

  printf("\n");
}


