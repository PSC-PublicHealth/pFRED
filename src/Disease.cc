/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Disease.cc
//

#include <stdio.h>
#include <new>
#include <string>
#include <sstream>

using namespace std;

#include "Disease.h"
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
#include "Transmission.h"
#include "Multistrain_Timestep_Map.h"
#include "Seasonality.h"
#include "Strain.h"

double Disease::R0 = -1.0;
double Disease::R0_a = -1.0;
double Disease::R0_b = -1.0;

Disease::Disease() {
  // note that the code that establishes the latent/asymptomatic/symptomatic
  // periods has been moved to the IntraHost class (or classes derived from it).
  transmissibility = -1.0;
  immunity_loss_rate = -1.0;
  mutation_prob = NULL;
  residual_immunity = NULL;
  infection_immunity_prob = NULL;
  vaccination_immunity_prob = NULL;
  at_risk = NULL;
  epidemic = NULL;
  population = NULL;
  strain_table = NULL;
}

Disease::~Disease() {
  delete epidemic;
  delete residual_immunity;
  delete infection_immunity_prob;
  delete vaccination_immunity_prob;
  delete at_risk;
  delete strain_table;
  delete ihm;
}

void Disease::setup(int disease, Population *pop, double *mut_prob) {
  id = disease;

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "disease %d setup entered\n", id);
    fflush(Global::Statusfp);
  }

  Params::get_param_from_string("R0",&R0);
  Params::get_param_from_string("R0_a",&R0_a);
  Params::get_param_from_string("R0_b",&R0_b);

  Params::get_indexed_param("trans",id,&transmissibility);
  Params::get_indexed_param("mortality_rate",id,&mortality_rate);
  Params::get_indexed_param("immunity_loss_rate",id,&immunity_loss_rate);

  if (Global::Enable_Climate) {
    Params::get_indexed_param("seasonality_multiplier_min",id,&seasonality_min);
    Params::get_indexed_param("seasonality_multiplier_max",id,&seasonality_max);
    Params::get_indexed_param("seasonality_multiplier_Ka",id,&seasonality_Ka); // Ka = -180 by default
    seasonality_Kb = log(seasonality_max - seasonality_min);
  }

  if (Disease::R0 > 0) {
    transmissibility = Disease::R0_a*Disease::R0*Disease::R0 + Disease::R0_b*Disease::R0;
  }

  mutation_prob = mut_prob;
  population = pop;

  // Read primary_cases file that indicates the number of external infections
  // that occur each day.
  // Note: infectees are chosen at random, and previously infected individuals
  // are not affected, so the number of new cases may be less than specified in
  // the file.
  char param_name[80];
  sprintf(param_name,"primary_cases[%d]",id);
  string param_name_str(param_name);
  Multistrain_Timestep_Map * msts = new Multistrain_Timestep_Map(param_name_str);
  msts->read_map();
  epidemic = new Epidemic(this, msts);

  // Define residual immunity
  residual_immunity = new Age_Map("Residual Immunity");
  residual_immunity->read_from_input("residual_immunity",id);

  // use params "Immunization_rate" to override the residual immunity for
  // the group starting with age 0.
  // This allows easy specification for a prior imunization rate, e.g.,
  // to specify 25% prior immunization rate, use the parameters:
  //
  // residual_immunity_ages[0] = 2 0 100
  // residual_immunity_values[0] = 1 0.0
  // Immunization_Rate = 0.25
  //
  double immunization_rate;
  Params::get_param_from_string("Immunization_Rate",&immunization_rate);
  if (immunization_rate >= 0.0) {
    residual_immunity->set_value(0, immunization_rate);
  }

  if(residual_immunity->is_empty() == false) residual_immunity->print();
 
  // Probability of developing an immune response by past infections
  infection_immunity_prob = new Age_Map( "Infection Immunity Probability" );
  infection_immunity_prob->read_from_input( "infection_immunity_prob", id );

  vaccination_immunity_prob = new Age_Map( "Vaccination Immunity Probability" );
  vaccination_immunity_prob->read_from_input( "vaccination_immunity_prob", id );
  
  // Define at risk people
  at_risk = new Age_Map("At Risk Population");
  at_risk->read_from_input("at_risk",id);

  // Initialize StrainTable
  strain_table = new StrainTable;
  strain_table->setup( this );
  strain_table->reset();

  // Initialize IntraHost
  int ihmType;
  Params::get_indexed_param("intra_host_model", id, &ihmType);
  ihm = IntraHost :: newIntraHost(ihmType);
  ihm->setup(this);

  // Initialize Infection Trajectory Thresholds
  Params::get_indexed_param( "infectivity_threshold", id, &infectivity_threshold ); 
  Params::get_indexed_param( "symptomaticity_threshold", id, &symptomaticity_threshold ); 

  int evolType;
  Params::get_indexed_param("evolution", id, &evolType);
  evol = EvolutionFactory :: newEvolution(evolType);
  evol->setup(this);

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

Disease * Disease::should_mutate_to() {
  int num_diseases = Global::Diseases;
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
  } else {
    days = -1;
  }

  return days;
}


int Disease::get_days_symp() {
  return ihm->get_days_symp();
}

double Disease::get_attack_ratio() {
  return epidemic->get_attack_ratio();
}

double Disease::get_symptomatic_attack_ratio() {
  return epidemic->get_symptomatic_attack_ratio();
}

// static
void Disease::get_disease_parameters() {
}

double Disease :: get_transmissibility(int strain) {
  return strain_table->get_transmissibility(strain);
}

Trajectory * Disease::get_trajectory( Infection *infection, Transmission::Loads * loads ) {
  return ihm->get_trajectory( infection, loads );
}

Transmission::Loads * Disease::get_primary_loads(int day) {
  return evol->get_primary_loads( day );
}
/// @brief Overloaded to allow specification of a single strain to be used for the initial loads.
Transmission::Loads * Disease::get_primary_loads(int day, int strain) {
  return evol->get_primary_loads( day, strain );
}

int Disease::get_max_days() {
  return ihm->get_max_days();
}

bool Disease::gen_immunity_infection( int age ) {
  double prob = infection_immunity_prob->find_value( age );
  return ( RANDOM() <= prob );
}

bool Disease::gen_immunity_vaccination( int age ) {
  double prob = vaccination_immunity_prob->find_value( age );
  return ( RANDOM() <= prob );
}

int Disease :: get_num_strain_data_elements( int strain ) {
  return strain_table->get_num_strain_data_elements( strain );
}

int Disease :: get_strain_data_element( int strain, int i ) {
  return strain_table->get_strain_data_element( strain, i );
}
 
void Disease::add_root_strain( int num_elements ) { strain_table->add_root_strain( num_elements ); }

int Disease::add_strain( Strain * child_strain, double transmissibility, int parent_strain_id ) {
  return strain_table->add( child_strain, transmissibility, parent_strain_id );
}

int Disease::add_strain( Strain * child_strain, double transmissibility ) {
  return strain_table->add( child_strain, transmissibility );
}

double Disease::calculate_climate_multiplier( double seasonality_value ) {
  return exp( ( ( seasonality_Ka * seasonality_value ) + seasonality_Kb ) ) + seasonality_min;
}

int Disease::get_num_strains() {
  return strain_table->get_num_strains();
}

void Disease::printStrain( int strain_id, stringstream & out ) {
  strain_table->printStrain( strain_id, out );
}

std::string Disease::get_strain_data_string( int strain_id ) {
  return strain_table->get_strain_data_string( strain_id );
}

const Strain_Data & Disease::get_strain_data( int strain ) {
  return strain_table->get_strain_data( strain );
}

const Strain & Disease::get_strain( int strain_id ) {
  return strain_table->get_strain( strain_id );
}

void Disease::initialize_evolution_reporting_grid( Large_Grid * grid ) {
  evol->initialize_reporting_grid( grid );
}

void Disease::init_prior_immunity() {
  evol->init_prior_immunity( this );
}

