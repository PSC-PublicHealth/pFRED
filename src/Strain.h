/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.h
//

#ifndef _FRED_STRAIN_H
#define _FRED_STRAIN_H

#include "Global.h"
#include "Epidemic.h"
#include <map>
#include <string>

using namespace std;

class Person;
class Disease;
class Age_Map;

class Strain {
public:
  Strain();
  UNIT_TEST_VIRTUAL ~Strain();
	
  UNIT_TEST_VIRTUAL void reset();
  UNIT_TEST_VIRTUAL void setup(int s, Disease *d);
  void setup(int strain, Disease *disease, map<string, double> *data, double trans);
  UNIT_TEST_VIRTUAL void print();

  UNIT_TEST_VIRTUAL int get_id() { return id; }
  UNIT_TEST_VIRTUAL double getTransmissibility() { return transmissibility; }
private:
  int id;
  double transmissibility;
  
  Disease *disease;
  map<string, double> *strainData;
};

#endif // _FRED_STRAIN_H
