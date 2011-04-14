/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Classroom.h
//

#ifndef _FRED_CLASSROOM_H
#define _FRED_CLASSROOM_H

#include "Place.h"

extern double * Classroom_contacts_per_day;
extern double *** Classroom_contact_prob;
extern char Classroom_closure_policy[];
extern int Classroom_closure_day;
extern double Classroom_closure_threshold;
extern int Classroom_closure_period;
extern int Classroom_closure_delay;
extern int Classroom_parameters_set;

class Classroom: public Place {
  
public: 
  Classroom() {}
  ~Classroom() {}
  Classroom(int, const char*,double,double,Place *, Population *pop);
  void get_parameters(int diseases);
  void enroll(Person * per);
  int get_group(int disease, Person * per);
  double get_transmission_prob(int disease, Person * i, Person * s);
  bool should_be_open(int day, int disease);
  double get_contacts_per_day(int disease);
  int get_age_level() { return age_level; }
 private:
  int age_level;
};

#endif // _FRED_CLASSROOM_H

