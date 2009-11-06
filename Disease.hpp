/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Disease.hpp
//

#ifndef _FRED_DISEASE_H
#define _FRED_DISEASE_H

class Disease {
public:
  void setup(int i);
  void print();
  int get_days_latent();
  int get_days_incubating();
  int get_days_infectious();
  double get_transmissibility() { return transmissibility; };
  double get_prob_symptomatic() { return prob_symptomatic; };

  static void get_disease_parameters();
  static void setup_diseases(int verbose);
  static int draw_from_distribution(int n, double *dist);
  static int get_diseases();
  static double get_beta(int dis);
  static double get_prob_symptomatic(int dis);
  static int get_days_latent(int dis);
  static int get_days_infectious(int dis);

private:
  int id;
  double prob_symptomatic;
  double prob_resistant;
  double transmissibility;
  int max_days_latent;
  int max_days_incubating;
  int max_days_infectious;
  double *days_latent;
  double *days_incubating;
  double *days_infectious;
};
  


#endif // _FRED_DISEASE_H
