/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Decision.cpp
//

#include "Policy.h"
#include "Decision.h"

Decision::Decision(Policy *p){
  policy = p;
  Name = "Generic Decision";
  Type = "Generic";
}


