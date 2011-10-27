/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Intention.cc
//

#include "Intention.h"
class Person;

Intention::Intention(Person * _person) {
  self = _person;
  type = REFUSE;
  probability = 0.0;
  frequency = 0;
  expiration = 0;
  willing = false;
}

