/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: School.h
//

#ifndef _FRED_SCHOOL_H
#define _FRED_SCHOOL_H

#include "Place.h"
#include <vector>

extern double * school_contacts_per_day;
extern double *** school_contact_prob;
extern char school_closure_policy[];
extern int school_closure_day;
extern double school_closure_threshold;
extern int school_closure_period;
extern int school_closure_delay;
extern int school_parameters_set;

class School : public Place {
public: 
  School() {}
  ~School() {}
  School(int,const char*,double,double,Place *, Population *pop);
  void prepare();
  void get_parameters(int diseases);
  int get_group(int disease, Person * per);
  double get_transmission_prob(int disease, Person * i, Person * s);
  int should_be_open(int day, int disease);
  double get_contacts_per_day(int disease);
  void enroll(Person * per);
  int children_in_grade(int age) {
    if (-1 < age && age < 20) return students_with_age[age];
    else return 0;
  }
  void print(int disease);
  void setup_classrooms();
  Place * assign_classroom(Person *per);

private:
  int students_with_age[20];
  vector <Place *> classrooms[20];
  int next_classroom[20];
  int next_classroom_without_teacher[20];
  int total_classrooms;
};

#endif // _FRED_SCHOOL_H
