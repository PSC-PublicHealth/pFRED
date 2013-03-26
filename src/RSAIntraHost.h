/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.
  RSA IntraHost class designed by Sarah Lukens

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

#ifndef _FRED_RSA_IntraHost_H
#define _FRED_RSA_IntraHost_H

#include <vector>
#include <map>

#include "IntraHost.h"
#include "Infection.h"
#include "Trajectory.h"
#include "Transmission.h"

class Infection;
class Trajectory;

class RSAIntraHost : public IntraHost {

  public:
    RSAIntraHost();

    /**
     * Get the infection Trajectory
     *
     * @param infection
     * @param loads
     * @return a pointer to a Trajectory object
     */
    Trajectory * get_trajectory( Infection * infection, Transmission::Loads * loads );
    /**
     * Set the attributes for the IntraHost
     *
     * @param dis the disease to which this IntraHost model is associated
     */
    void setup(Disease *disease);
	/**
     * @return the prob_symptomatic
     */
    double get_prob_symptomatic() {
      return prob_symptomatic;
    }
    /* Sarah functions: */
	/**
     * Get random number in [-scale, scale]
     *
     * @param scale
     * @return random number in [-scale, scale]
     */
    double random_phenotypic_value(double scale) ;
    /**
     * Get symptom score
     *
     * @param x1
     * @param x2
     * @param day
     * @return Symptom score at day day as a function of x1,x2
     */
    double Get_symptoms_score(double x1, double x2, int day);
    /**
     * Get symptom score
     *
     * @param x1
     * @param x2
     * @param day
     * @return Infectivity at day day as a function of x1,x2
     */
    double Get_Infectivity(double x1, double x2, int day);
     /**
     * Map agent's age to an input value for RSA evaluation
     *
     * @param age
     * @return age mapped from [18, 65] to [-scale, scale]
     */
    double map_age_value(double age, double scale);
    
        private:
        static const int MAX_LENGTH = 10;
        int days_sick;
        double prob_symptomatic;

  };

#endif