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
#include "Vaccine_Dose.h"

Vaccine::Vaccine(string _name, int _id, int _strain, int _age0, int _age1, 
		 int _total_avail, int _additional_per_day, int _start_day){
  name =               _name;
  id =                 _id;
  strain =             _strain;
  ages[0] =            _age0;
  ages[1] =            _age1;
  additional_per_day = _additional_per_day;
  start_day =          _start_day;
  
  initial_stock = 0;
  stock = 0;
  reserve = _total_avail;
  total_avail = _total_avail;
  number_delivered = 0;
  number_effective = 0;
}

Vaccine::~Vaccine(void){ 
  for(unsigned int i = 0; i < doses.size(); i++) delete doses[i];
}
    
void Vaccine::add_dose(Vaccine_Dose* _vaccine_dose) {
  doses.push_back(_vaccine_dose);
}

void Vaccine::print(void) {
  cout << "Name = \t\t\t\t" <<name << "\n";
  cout << "Applied to strain = \t\t" << strain << "\n";
  cout << "Initial Stock = \t\t" << initial_stock << "\n";
  cout << "Total Available = \t\t"<< total_avail << "\n";
  cout << "Amount left to system = \t" << reserve << "\n";
  cout << "Additional Stock per day =\t" << additional_per_day << "\n";
  cout << "Starting on day = \t\t" << start_day << "\n";
  cout << "Dose Information\n";
  for(unsigned int i=0;i<doses.size();i++){
    cout <<"\t" << setw(20) << "Dose #"<<i+1 << "\n";
    doses[i]->print();
  }
}

void Vaccine::reset(void) {
  stock = 0;
  reserve = total_avail;
}
 
void Vaccine::update(int day) {
  if(day >= start_day) add_stock(additional_per_day);
}
