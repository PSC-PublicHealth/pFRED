/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Vaccines.cc
//


#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

#include "Vaccines.h"
#include "Vaccine.h"
#include "Vaccine_Dose.h"
#include "Random.h"
#include "Age_Map.h"

void Vaccines::setup(string _vaccine_file) {
  
  ifstream vaccine_input;
  vaccine_input.open(_vaccine_file.c_str());
  
  if(!vaccine_input.is_open()){
    cout << "Help!Vaccines input file " << _vaccine_file << " cannot be open\n";
    abort();
  }
  
  string lline;
  getline(vaccine_input,lline);
  istringstream vstr(lline);
  
  // Get Comment line
  getline(vaccine_input,lline);
  
  int number_vacc;
  vstr >> number_vacc;
  
  for(int iv=0;iv<number_vacc;iv++) {
    int la;
    int ha;
    int ta;
    int apd;
    int std;
    int num_doses;
    
    getline(vaccine_input,lline);
    istringstream vvstr(lline);
    vvstr >> la >> ha >> num_doses >> ta >> apd >> std;
    
    stringstream name;
    name << "Vaccine#"<<iv+1;
    vaccines.push_back(new Vaccine(name.str(),iv,0,la,ha,ta,apd,std));
    
    for(int id=0;id<num_doses;id++) {
      
      Age_Map* efficacy_map = new Age_Map("Dose Efficacy");
      Age_Map* efficacy_delay_map = new Age_Map("Dose Delay");
      
      int naggroups;
      int tbd;
      double redi;
      
      // Read in the first dose line
      getline(vaccine_input,lline);
      istringstream dstr(lline);
      
      dstr >> naggroups >> redi >> tbd;
      //cout << "dstr = "<< naggroups << " " << redi << " " << tbd << "\n";
      for(int ig=0;ig<naggroups;ig++) {
        int age1, age2;
        double eff, effd;
        
        // read in dose efficacy line
        getline(vaccine_input,lline);
        istringstream ddstr(lline);
        
        ddstr >> age1 >> age2 >> eff >> effd;
        
        efficacy_map->add_value(age1,age2,eff);
        efficacy_delay_map->add_value(age1,age2,effd);
      }
      //Vaccine_Dose vd(effmap,effdmap,tbd);
      vaccines[iv]->add_dose(new Vaccine_Dose(efficacy_map,efficacy_delay_map,tbd));
    }
  }
}	

void Vaccines::print() const {
  cout <<"Vaccine Package Information\n";
  cout <<"There are "<<vaccines.size() <<" vaccines in the package\n";
  fflush(stdout);
  for(unsigned int i=0;i<vaccines.size(); i++){
    vaccines[i]->print();
  }
  fflush(stdout);
}

void Vaccines::print_current_stocks() const {
  cout << "Vaccine Stockk Information\n";
  cout << "\nVaccines# " << "Current Stock      " << "Current Reserve    \n";
  for(unsigned int i=0; i<vaccines.size(); i++) {
    cout << setw(10) << i+1 << setw(20) << vaccines[i]->get_current_stock()
    << setw(20) << vaccines[i]->get_current_reserve() << "\n";
  }
}

void Vaccines::update(int day) {
  for(unsigned int i=0;i<vaccines.size(); i++) {
    vaccines[i]->update(day);
  }
}

void Vaccines::reset() {
  for(unsigned int i=0;i<vaccines.size();i++) {
    vaccines[i]->reset();
  }
}


int Vaccines::pick_from_applicable_vaccines(int age) const {
  vector <int> app_vaccs;
  for(unsigned int i=0;i<vaccines.size();i++){
    // if first dose is applicable, add to vector.
    if(vaccines[i]->get_dose(0)->is_within_age(age) &&
       vaccines[i]->get_current_stock() > 0){
      app_vaccs.push_back(i);
    }
  }
  
  if(app_vaccs.size() == 0){ return -1; }
  
  int randnum = 0;
  if(app_vaccs.size() > 1){
    randnum = (int)(RANDOM()*app_vaccs.size());
  }
  return app_vaccs[randnum];
}

int Vaccines::get_total_vaccines_avail_today() const {
  int total=0;
  for(unsigned int i=0;i<vaccines.size();i++){
    total += vaccines[i]->get_current_stock();
  }
  return total;
}
