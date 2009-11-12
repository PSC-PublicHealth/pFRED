/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.hpp
//

#ifndef _FRED_STRAIN_H
#define _FRED_STRAIN_H

class Strain {
public:
  void setup(int i);
  void print();
  int get_days_latent();
  int get_days_incubating();
  int get_days_infectious();
  double get_transmissibility() { return transmissibility; };
  double get_prob_symptomatic() { return prob_symptomatic; };

  static void get_strain_parameters();
  static void setup_strains(int verbose);
  static int draw_from_straintribution(int n, double *straint);
  static int get_strains();
  static double get_beta(int strain);
  static double get_prob_symptomatic(int strain);
  static int get_days_latent(int strain);
  static int get_days_infectious(int strain);
  static double get_prob_stay_home();

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
  


#endif // _FRED_STRAIN_H
