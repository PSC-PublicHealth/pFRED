/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Disease.cc
//

#include "Disease.h"

#include <stdio.h>
#include <new>
#include <string>
#include <sstream>

using namespace std;

#include "Global.h"
#include "Params.h"
#include "Population.h"
#include "Random.h"
#include "Age_Map.h"
#include "Epidemic.h"
#include "Timestep_Map.h"
#include "StrainTable.h"
#include "IntraHost.h"
#include "Evolution.h"
#include "EvolutionFactory.h"

double Prob_stay_home;

Disease::Disease() {
  // note that the code that establishes the latent/asymptomatic/symptomatic
  // periods has been moved to the IntraHost class (or classes derived from
  // it.
  transmissibility = -1.0;
  immunity_loss_rate = -1.0;
  mutation_prob = NULL;
  residual_immunity = NULL;
  at_risk = NULL;
  epidemic = NULL;
  population = NULL;
  strainTable = NULL;
}

Disease::~Disease() {
  delete epidemic;
  delete residual_immunity;
  delete at_risk;
  delete strainTable;
  delete ihm;
}

void Disease::reset() {
  clear();
}

void Disease::clear() {
  epidemic->clear();
  strainTable->reset();
}

void Disease::setup(int disease, Population *pop, double *mut_prob) {
  id = disease;

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "disease %d setup entered\n", id);
    fflush(Global::Statusfp);
  }

  get_indexed_param("trans",id,&transmissibility);
  get_indexed_param("mortality_rate",id,&mortality_rate);
  get_indexed_param("immunity_loss_rate",id,&immunity_loss_rate);

  // This needs to be moved to Behavior
  get_param((char *) "prob_stay_home", &Prob_stay_home);
  mutation_prob = mut_prob;
  population = pop;

  // Read primary_cases file that indicates the number of externally infections
  // occur each day.
  // Note: infectees are chosen at random, and previously infected individuals
  // are not affected, so the number of new cases may be less than specified in
  // the file.
  char param_name[80];
  sprintf(param_name,"primary_cases[%d]",id);
  string param_name_str(param_name);
  epidemic = new Epidemic(this, new Timestep_Map(param_name_str));

  // Define residual immunity
  residual_immunity = new Age_Map("Residual Immunity");
  residual_immunity->read_from_input("residual_immunity",id);

  if(residual_immunity->is_empty() == false) residual_immunity->print();

  // Define at risk people
  at_risk = new Age_Map("At Risk Population");
  at_risk->read_from_input("at_risk",id);

  // Initialize StrainTable
  strainTable = new StrainTable;
  strainTable->setup(this);

  // Initialize IntraHost
  int ihmType;
  get_indexed_param("intra_host_model", id, &ihmType);
  ihm = IntraHost :: newIntraHost(ihmType);
  ihm->setup(this);

  int evolType;
  get_indexed_param("evolution", id, &evolType);
  evol = EvolutionFactory :: newEvolution(evolType);

  if(at_risk->is_empty() == false ) at_risk->print();

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "disease %d setup finished\n", id);
    fflush(Global::Statusfp);
    print();
  }
}

void Disease::print() {
  // Since most of Disease has been moved to IntraHost (or classes derived from it)
  // the old print() function no longer worked.
  // TODO: write new print() statement or remove
}

Disease* Disease::should_mutate_to() {
  int num_diseases = population->get_diseases();
  // Pick a random index to consider mutations from, so that mutating to all diseases is
  // dependent only on muation_prob, and not on the order of the diseases.
  int disease_start = IRAND(0, num_diseases-1);
  // By default, create an infection with this disease.
  int infection_id = id;

  // Try and mutate to other diseases.
  for (int disease_i = disease_start; disease_i < num_diseases; ++disease_i) {
    if (disease_i == id) continue;

    double r = RANDOM();

    if (r < mutation_prob[disease_i]) {
      return population->get_disease(disease_i);
    }
  }

  if (infection_id == id) {
    // Didn't mutate yet.
    for (int disease_i = 0; disease_i < disease_start; ++disease_i) {
      if (disease_i == id) continue;

      double r = RANDOM();

      if (r < mutation_prob[disease_i]) {
        return population->get_disease(disease_i);
      }

    }
  }

  return NULL;
}

int Disease::get_days_recovered() {
  int days;

  if (immunity_loss_rate > 0.0) {
    // draw from exponential distribution
    days = floor(0.5 + draw_exponential(immunity_loss_rate));
    // printf("DAYS RECOVERED = %d\n", days);
  }
  else {
    days = -1;
  }

  return days;
}


int Disease::get_days_symp() {
  return ihm->get_days_symp();
}

double Disease::get_attack_rate() {
  return epidemic->get_attack_rate();
}

void Disease::update_stats(int day) {
  epidemic->update_stats(day);
}

void Disease::print_stats(int day) {
  epidemic->print_stats(day);
}

// static
double Disease::get_prob_stay_home() {
  return Prob_stay_home;
}

// static
void Disease::set_prob_stay_home(double p) {
  Prob_stay_home = p;
}

// static
void Disease::get_disease_parameters() {
}

void Disease::update(Date *sim_start_date, int day) {
  epidemic->update(sim_start_date, day);
}

double Disease :: get_transmissibility(int strain) {
  return strainTable->getTransmissibility(strain);
}

Trajectory *Disease :: getTrajectory(Infection *infection, map<int, double> *loads) {
  return ihm->getTrajectory(infection, loads);
}

map<int, double> *Disease :: getPrimaryLoads(int day) {
  return evol->getPrimaryLoads(day);
}

UNIT_TEST_VIRTUAL int Disease :: get_max_days() {
  return ihm->get_max_days();
}
