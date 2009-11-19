/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Demographics.cpp
//

#include "Demographics.h"
#include "Person.h"
#include "Random.h"

Demographics::Demographics(int a, char s, char occ, int mar, int prof) {
  init_age = a;
  sex = s;
  init_occupation = occ;
  init_marital_status = mar;
  init_profession = prof;
  birthday = -(init_age*365+IRAND(0,364));
  reset();
}

void Demographics::update(int day) {
}

void Demographics::reset() {
  age = init_age;
  occupation = init_occupation;
  marital_status = init_marital_status;
  profession = init_profession;
}

void Demographics::set_occupation() {
  // set occupation by age
  if (age < 5) { occupation = 'C'; }
  else if (age < 19) { occupation = 'S'; }
  else if (age < 65) { occupation = 'W'; }
  else { occupation = 'R'; }
}

void Demographics::print() {
}

int Demographics::get_age(int day) { 
  return (int) ((day-birthday)/365.0);
}
