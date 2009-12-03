/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Perceptions.h
//

#ifndef _FRED_PERCEPTIONS_H
#define _FRED_PERCEPTIONS_H

class Person;

class Perceptions {
public:
  Perceptions(Person *p);
  void reset();
  void update(int day);

private:
  Person * me;
  double perceived_susceptibility;
  double perceived_severity;
  double perceived_benefits;
  double perceived_barriers;
  double self_efficacy;
};

#endif // _FRED_PERCEPTIONS_H

