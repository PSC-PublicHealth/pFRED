/*
 *  Timestep_Map.cpp
 *  FRED
 *
 *  Created by Shawn Brown on 4/30/10.
 *  Copyright 2010 University of Pittsburgh. All rights reserved.
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include "Timestep_Map.h"
#include "Params.h"

using namespace std;

Timestep_Map::Timestep_Map(void){
  values = NULL;
  name = "";
}

Timestep_Map::Timestep_Map(string _name){
  name = _name;
  
  
  char map_file_param[255];
  char map_file_name[255];
  // Need Special parsing if this is an array from input
  // Allows for Strain specific values
  if(name.find("[") != string::npos) {
    string name_tmp;
    string number;
    size_t found = name.find_last_of("[");
    size_t found2 = name.find_last_of("]");
    name_tmp.assign(name.begin(),name.begin()+found);
    number.assign(name.begin()+found+1,name.begin()+found2);
    sprintf(map_file_param,"%s_file[%s]",name_tmp.c_str(),number.c_str());
  }
  else {
    sprintf(map_file_param,"%s_file",name.c_str());
  }
  
  // Read the filename from params
  get_param(map_file_param,map_file_name);
  
  // If this parameter is "none", then there is no map
  if(strncmp(map_file_name,"none",4)==0){
    fflush(stdout);
    values = NULL;
    return;
  }
 
  ifstream ts_input;
  
  ts_input.open(map_file_name);
  if(!ts_input.is_open()) {
    cout << "Help!  Can't read " << map_file_name << " Timestep Map\n";
    abort();
  }
  
  // There is a file, lets read in the data structure.
  values = new map<int,int>;
  string line;
  while(getline(ts_input,line)){
    int ts, value;
    istringstream istr(line);
    istr >> ts >> value;
    values->insert(pair<int,int>(ts,value));
  }
}

Timestep_Map::~Timestep_Map(void){
  if(values!= NULL){
    delete values;
  }
}

int Timestep_Map::get_value_for_timestep(int ts){
  map<int,int>::iterator itr;
  
  itr = values->find(ts);
  if(itr != values->end()){
    return itr->second;
  }
  
  return 0;
}
	  
void Timestep_Map::print(void){
  cout << "\n";
  cout << name << " Timestep Map  " << values->size() <<"\n";
  map<int,int>::iterator itr;
  for(itr=values->begin(); itr!=values->end(); ++itr){
    cout << setw(5) << itr->first << ": " << setw(10) << itr->second << "\n";
  }
  cout << "\n";
}

	
	
	
		
	   
