/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Disease.hpp
//

#ifndef _SYNDEM_DISEASE_H
#define _SYNDEM_DISEASE_H

#include <stdio.h>
#include <new>
using namespace std;

#include "Random.hpp"
#include "Params.hpp"

class Disease {
  int id;
  double prob_symptomatic;
  double transmissibility;
  int max_days_latent;
  int max_days_incubating;
  int max_days_infectious;
  double *days_latent;
  double *days_incubating;
  double *days_infectious;

public:
  void setup(int i);
  void print();
  int get_days_latent();
  int get_days_incubating();
  int get_days_infectious();
  double get_transmissibility() { return transmissibility; };
  double get_prob_symptomatic() { return prob_symptomatic; };
};
  
void get_disease_parameters();
void setup_diseases();
int draw_from_distribution(int n, double *dist);


#endif // _SYNDEM_DISEASE_H
