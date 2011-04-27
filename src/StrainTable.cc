/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: StrainTable.cc
//

#include "StrainTable.h"
#include "Params.h"
#include "Disease.h"
#include "Strain.h"
#include "Global.h"
#include <vector>
#include <map>

using namespace std;

StrainTable :: StrainTable() {
  strains = NULL;
  originalStrains = 1;
  }

StrainTable :: ~StrainTable() {
  delete strains;
  }

void StrainTable :: setup(Disease *d) {
  disease = d;
  int diseaseId = disease->get_id();

  int numStrains = 1;
  get_indexed_param((char *) "num_strains", diseaseId, &numStrains);
  originalStrains = numStrains;

  if(Verbose) printf("Reading %d strains for disease %d\n", numStrains, diseaseId);

  strains = new vector<Strain *>();

  for(int is = 0; is < numStrains; is++) {
    Strain *s = new Strain();
    s->setup(is, disease);
    strains->push_back(s);
    }
  }

void StrainTable :: reset() {
  strains->clear();
  setup(disease);
  }

void StrainTable :: add(Strain *s) {
  strains->push_back(s);
  }

double StrainTable :: getTransmissibility(int id) {
  return strains->at(id)->getTransmissibility();
  }
