/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Random_Behavior_Model.cc
//

#include "Random_Behavior_Model.h"
#include "Person.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"


#include <stdio.h>
#include <iostream>

using namespace std;

Random_Behavior_Model::Random_Behavior_Model(Person *p) {
  self = p;
}


