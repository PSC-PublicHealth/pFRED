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
#include "VaccineDose.h"
#include "Random.h"

Vaccines::Vaccines(string vaccFile){
  
  ifstream vaccinput;
  vaccinput.open(vaccFile.c_str());
  
  if(!vaccinput.is_open()){
    cout << "\nHelp!Vaccines input file " << vaccFile << " cannot be open";
    abort();
  }
  
  string lline;
  getline(vaccinput,lline);
  istringstream vstr(lline);
  
  // Get Comment line
  getline(vaccinput,lline);
  
  int number_vacc;
  vstr >> number_vacc;
  
  for(int iv=0;iv<number_vacc;iv++){
    int la;
    int ha;
    int ta;
    int apd;
    int std;
    int num_doses;
    
    getline(vaccinput,lline);
    istringstream vvstr(lline);
    vvstr >> la >> ha >> num_doses >> ta >> apd >> std;
    
    stringstream name;
    name << "Vaccine#"<<iv+1;
    cout <<"\nta = " << ta;
    vaccines.push_back(Vaccine(name.str(),iv,0,la,ha,ta,apd,std));
    
    for(int id=0;id<num_doses;id++){
      
      AgeMap effmap("Dose Efficacy");
      AgeMap effdmap("Dose Delay");
      
      int naggroups;
      int tbd;
      int redi;
      
      // Read in the first dose line
      getline(vaccinput,lline);
      istringstream dstr(lline);
      
      dstr >> naggroups >> redi >> tbd;
      
      for(int ig=0;ig<naggroups;ig++){
	int age1, age2;
	double eff, effd;
	
	// read in dose efficacy line
	getline(vaccinput,lline);
	istringstream ddstr(lline);
	
	ddstr >> age1 >> age2 >> eff >> effd;
	
	effmap.add_value(age1,age2,eff);
	effdmap.add_value(age1,age2,effd);
      }
      Vaccine_Dose vd(effmap,effdmap,tbd);
      vaccines[iv].Add_dose(vd);
    }
  }
}	

void Vaccines::print(void){
  cout <<"\nVaccine Package Information";
  cout <<"\nThere are "<<vaccines.size() <<" vaccines in the package";
  fflush(stdout);
  for(unsigned int i=0;i<vaccines.size(); i++){
    vaccines[i].print();
  }
  fflush(stdout);
  //abort();
}

void Vaccines::print_current_stocks(void){
  cout << "\nVaccine Stockk Information";
  cout << "\n\nVaccines# " << "Current Stock      " << "Current Reserve    ";
  for(unsigned int i=0; i<vaccines.size(); i++){
    cout << "\n" << setw(10) << i+1 << setw(20) << vaccines[i].get_current_stock()
	 << setw(20) << vaccines[i].get_current_reserve();
  }
  cout << "\n";
}

void Vaccines::update(int day){
  for(unsigned int i=0;i<vaccines.size(); i++){
    vaccines[i].update(day);
  }
}

void Vaccines::reset(void){
  for(unsigned int i=0;i<vaccines.size();i++){
    vaccines[i].reset();
  }
}


int Vaccines::pick_from_applicable_vaccines(int age){
  vector <int> app_vaccs;
  for(unsigned int i=0;i<vaccines.size();i++){
    // if first dose is applicable, add to vector.
    if(vaccines[i].get_dose(0)->is_within_age(age) &&
       vaccines[i].get_current_stock() > 0){
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
 
int Vaccines::get_total_vaccines_avail_today(void){
  int total=0;
  for(unsigned int i=0;i<vaccines.size();i++){
    total += vaccines[i].get_current_stock();
  }
  return total;
}
