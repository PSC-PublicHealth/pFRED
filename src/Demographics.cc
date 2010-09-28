/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Demographics.cc
//

#include "Demographics.h"
#include "Population.h"
#include "Age_Map.h"
#include "Person.h"
#include "Random.h"

Demographics::Demographics() {
  self = NULL;
  init_age = -1;
  age = -1;
  sex = 'n';
  init_marital_status = -1;
  marital_status = -1;
  init_profession = -1;
  profession = -1;
  birthday = -1;
  pregnant = false;
}

Demographics::Demographics(Person* _self, int _age, char _sex,
                           int _marital_status,int _profession) {
  self                = _self;
  init_age            = _age;
  sex                 = _sex;
  init_marital_status = _marital_status;
  init_profession     = _profession;
  birthday            = -(init_age*365+IRAND(0,364));
  pregnant            = false;
  reset();
}

Demographics::~Demographics() {
}

void Demographics::update(int day) {
}

void Demographics::reset() {
  age                = init_age;
  marital_status     = init_marital_status;
  profession         = init_profession;
  pregnant            = false;
  if(sex == 'F'){
    Age_Map* preg_prob_map = self->get_population()->get_pregnancy_prob();
    if(preg_prob_map->get_num_ages() > 0){
      double preg_prob = preg_prob_map->find_value(age);
      if(RANDOM() < preg_prob)   pregnant = true;
    }
  }
}

void Demographics::print() {
}

int Demographics::get_age(int day) { 
  return  init_age;
  return (int) ((day-birthday)/365.0);
}
