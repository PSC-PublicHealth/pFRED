/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: StrainTable.h
//

#ifndef _FRED_StrainTable_H
#define _FRED_StrainTable_H

#include <vector>
#include <map>

class Strain;
class Disease;

using namespace std;

class Disease;
class Strain;

class StrainTable {
public:
  StrainTable();
  ~StrainTable();
  
  void setup(Disease *d); // Initial strains
  void reset();

  void add(Strain *s);
  double getTransmissibility(int id);

private:
  Disease *disease;
  vector<Strain *> *strains; 
  int originalStrains;
};

#endif // _FRED_StrainTable_H
