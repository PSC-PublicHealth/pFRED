/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Vaccine.cc
//
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

#include "Vaccine.h"
#include "VaccineDose.h"

Vaccine::Vaccine(string nm, int id, int str, int low_age, int high_age, int totav, int addpday, int stday){
  Name = nm;
  ID = id;
  strain = str;
  ages[0] = low_age;
  ages[1] = high_age;
  additional_per_day = addpday;
  start_day = stday;
  
  initial_stock = 0;
  stock = 0;
  reserve = totav;
  total_avail = totav;
  number_delivered = 0;
  number_effective = 0;
}

void Vaccine::Add_dose(Vaccine_Dose vd){
  Doses.push_back(vd);
}


void Vaccine::print(void){
  cout << "\nName = \t\t\t\t" <<Name;
  cout << "\nApplied to strain = \t\t" << strain;
  cout << "\nInitial Stock = \t\t" << initial_stock;
  cout << "\nTotal Available = \t\t"<< total_avail;
  cout << "\nAmount left to system = \t\t" << reserve;
  cout << "\nAdditional Stock per day =\t" << additional_per_day;
  cout << "\nStarting on day = \t\t" << start_day;
  cout << "\nDose Information";
  for(unsigned int i=0;i<Doses.size();i++){
    cout <<"\n\t" << setw(20) << "Dose #"<<i+1;
    Doses[i].print();
  }
  cout <<"\n\n";
}

void Vaccine::reset(void) {
  stock = 0;
  reserve = total_avail;
}

void Vaccine::update(int day){
  if(day >= start_day) add_stock(additional_per_day);
}
