/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: School.h
//

#ifndef _FRED_SCHOOL_H
#define _FRED_SCHOOL_H

#include <vector>

#include "Place.h"
#include "Random.h"


#define GRADES 20

class Classroom;

class School : public Place {
public: 
  School() {}
  ~School() {}
  School( const char*, double, double, Place *, Population *pop );
  void prepare();
  void get_parameters(int diseases);
  int get_group(int disease_id, Person * per);
  double get_transmission_prob(int disease_id, Person * i, Person * s);
  bool should_be_open(int day, int disease_id);
  void apply_global_school_closure_policy(int day, int disease_id);
  void apply_individual_school_closure_policy(int day, int disease_id);
  double get_contacts_per_day(int disease_id);
  void enroll(Person * per);
  void unenroll(Person * per);
  int children_in_grade(int age) {
    if (0 <= age && age < GRADES)
      return students_with_age[age];
    else return 0;
  }
  int classrooms_for_age(int age) {
    if (age < 0 || GRADES <= age) return 0;
    return (int) classrooms[age].size();
  }
  void print(int disease);
  int get_number_of_rooms();
  // int get_number_of_classrooms() { return (int) classrooms.size(); }
  void setup_classrooms( Allocator< Classroom > & classroom_allocator );
  Place * select_classroom_for_student(Person *per);
  int get_staff_size() { return staff_size; }

private:
  static double *** school_contact_prob;
  static char school_closure_policy[];
  static int school_closure_day;
  static double school_closure_threshold;
  static double individual_school_closure_threshold;
  static int school_closure_cases;
  static int school_closure_period;
  static int school_closure_delay;
  static bool school_parameters_set;
  static int school_summer_schedule;
  static char school_summer_start[];
  static char school_summer_end[];
  static int school_classroom_size;
  static double * school_contacts_per_day;
  static bool global_closure_is_active;
  static int global_close_date;
  static int global_open_date;

  int students_with_age[GRADES];
  int next_classroom[GRADES];
  vector <Place *> classrooms[GRADES];
  bool closure_dates_have_been_set;
  int staff_size;
};

#endif // _FRED_SCHOOL_H
