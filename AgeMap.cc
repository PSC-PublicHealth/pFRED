/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AgeMap.cpp
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "AgeMap.h"
#include "Params.h"

AgeMap::AgeMap(void){}

AgeMap::AgeMap(string name){
  Name = name + " Age Map";
  //ages = new vector<int>(0);
  //values= new vector<double>(0);
}

int AgeMap::read_from_input(string Input){
  // Need to fix this so I can use strings.
  //  string ages_string = Input + "_ages";
  //string values_string = Input + "_values";
  
  Name = Input + " Age Map";

  char ages_string[255];
  char values_string[255];
  
  //cout << "\nInput "<<Input;
  if(Input.find("[") != string::npos){
    string input_tmp;
    string number;
    size_t found = Input.find_last_of("[");
    size_t found2 = Input.find_last_of("]");
    input_tmp.assign(Input.begin(),Input.begin()+found);
    number.assign(Input.begin()+found+1,Input.begin()+found2);
    //cout << "\nStrings = "<<input_tmp << " " << number << "\n";
    sprintf(ages_string,"%s_ages[%s]",input_tmp.c_str(),number.c_str());
    sprintf(values_string,"%s_values[%s]",input_tmp.c_str(),number.c_str());
  }
  else{
    sprintf(ages_string,"%s_ages",Input.c_str());
    sprintf(values_string,"%s_values",Input.c_str());
  }
  //cout << "\n strings " << ages_string << " " << values_string;
  //exit(0);
  vector < int > ages_tmp;
  int na = get_param_vector(ages_string,ages_tmp);
  for(int i=0;i<ages_tmp.size();i++){
    cout <<"\n Ages "<<i<< " = "<< ages_tmp[i];
  }
  
  if(na % 2) {
    cout <<"\nError parsing AgeMap: " << Input << ": Must be an even number of age entries";
    abort();
  }

  for(int i=0;i<na; i+=2){
    vector <int> ages_tmp2(2);
    ages_tmp2[0] = ages_tmp[i];
    ages_tmp2[1] = ages_tmp[i+1];
    ages.push_back( ages_tmp2 );
  }
  
  int nv = get_param_vector(values_string,values);
   
  if(na != nv*2 ) {
    cout << "\nError parsing AgeMap: "<< Input << ": Inconsistent vector sizes";
    return -1;
  }
  
  // Check Ages
  if(na > 1){
    for(unsigned int i=0; i<ages.size(); i++){
      if(ages[i][0] > ages[i][1]){
	cout <<"\nError parsing AgeMap: "<< Input << ": Age "<<ages[i][0] << "and "<<ages[i][1] << " at position "
	     << i;
	return -1;
      }
    }
  }
  return 0;
}

void AgeMap::add_value(int lower_age, int upper_age,double val){
  vector < int > ages_tmp(2);
  ages_tmp[0] = lower_age;
  ages_tmp[1] = upper_age;
  ages.push_back(ages_tmp);
  values.push_back(val);  
}


double AgeMap::find_value(int age){

  //  if(age >=0 && age < ages[0]) return values[0];
  for(unsigned int i=0;i<values.size();i++)
    if(age >= ages[i][0] && age <= ages[i][1])
      return values[i];
  
  return 0.0;
}


void AgeMap::print(void){
  cout << "\n" << Name;
  cout << "\n--------------------------------------------------------";
  //cout << "\n"<< setw(8) << "0" << setw(8) << ages[0]-1 << setw(10) << setprecision(2) << values[0];
  for(unsigned int i=0;i<ages.size();i++){
    cout <<"\n"<< setw(8) << ages[i][0] << setw(8) << ages[i][1] << setw(10) << setprecision(2) << values[i];
  }
  
  cout << "\n--------------------------------------------------------\n";

}
