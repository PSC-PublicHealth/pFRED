/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Policy.cpp
//
#include <iostream>
#include <iomanip>
#include <vector>
#include "Policy.h"
#include "Decision.h"
#include "Manager.h"

Policy::Policy(void) {
  Name = "";
  manager = NULL;
}

Policy::~Policy(void) { }

Policy::Policy(Manager* mgr){
  Name = "Generic";
  manager = mgr;
}

int Policy::choose(Person* person, int strain, int current_day){
  int result=-1;
  for(unsigned int i=0; i < Decision_list.size(); i++){
    
    int new_result = Decision_list[i]->evaluate(person,strain,current_day);
    if(new_result == -1) return -1;
    else if(new_result > result) result = new_result;
    cout <<"\nResult for decision "<<i<< " is "<< result;
  }
  return result;
}   

void Policy::print(void){
  cout << "\nPolicy List for Decision "<< Name;
  cout << "\n\n" << setw(40) << "Policy " << setw(20) << "Type";
  cout << "\n------------------------------------------------------------------\n";
  for(unsigned int i=0;i<Decision_list.size(); ++i){
    cout << setw(40) << Decision_list[i]->get_name() << setw(20) << Decision_list[i]->get_type() << "\n";
  } 
}

void Policy::reset(void) { 
  cout << "Decision Reset not implemented yet";
}
