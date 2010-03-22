/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: VaccineDose.cc
//

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "VaccineDose.h"

Vaccine_Dose::Vaccine_Dose(AgeMap &eff, AgeMap &effdel, int dbd){
  Efficacy = eff;
  EfficacyDelay = effdel;
  DaysBetweenDoses = dbd;
}

void Vaccine_Dose::print(void){
  cout << "\nTime Between Doses:\t " << DaysBetweenDoses;
  Efficacy.print();
  EfficacyDelay.print();
}

int Vaccine_Dose::is_within_age(int age){
  if(Efficacy.find_value(age) != 0.0){
    return 1;
  }
}
