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
// File: StrainTable.cc
//

#include <vector>
#include <map>

#include "StrainTable.h"
#include "Params.h"
#include "Disease.h"
#include "Strain.h"
#include "Global.h"
#include "Utils.h"

using namespace std;

StrainTable::StrainTable() {
  strains.clear();
}

StrainTable::~StrainTable() { }

void StrainTable::setup( Disease * d ) {
  disease = d;
  int diseaseId = disease->get_id();
  strains.clear();
}

void StrainTable::add_root_strain( int num_elements ) {
  assert( strains.size() == 0 );
  Strain * new_strain = new Strain( num_elements );
  new_strain->setup( 0, disease, disease->get_transmissibility(), NULL );
  strains.push_back( new_strain );
}

void StrainTable::reset() {
  strains.clear();
  setup( disease );
}

void StrainTable::add( Strain * strain ) {
  strains.push_back( strain );
}

int StrainTable::add( Strain * child_strain, double transmissibility, int parent_strain_id ) {

  fred::Spin_Lock lock( mutex );
 
  int child_strain_id = parent_strain_id;
  Strain * parent_strain = strains[ parent_strain_id ];
  // if no change, return the parent strain id
  if ( child_strain->get_data() == parent_strain->get_data() ) {
    return child_strain_id;
  }
  // if this genotype has been seen already, re-use existing id
  std::string child_geno_string = child_strain->to_string(); 
  if ( strain_genotype_map.find( child_geno_string ) == strain_genotype_map.end() ) {
    child_strain_id = strain_genotype_map[ child_geno_string ];
  }
  else {
    // child strain id is next available id from strain table
    child_strain_id = strains.size();
    strain_genotype_map[ child_geno_string ] = child_strain_id;
    // set the child strain's id, disease pointer, transmissibility, and parent strain pointer
    child_strain->setup( child_strain_id, disease, transmissibility, parent_strain );
    // Add the new child to the strain table
    add( child_strain );
  }
  // return the newly created id
  return child_strain_id;
}

double StrainTable::get_transmissibility( int id ) {
  return strains[ id ]->get_transmissibility();
}

int StrainTable :: get_num_strain_data_elements(int strain) {
  //if(strain >= strains.size()) return 0;
  return strains[ strain ]->get_num_data_elements();
}

const Strain_Data & StrainTable::get_strain_data( int strain ) {
  return strains[ strain ]->get_strain_data();
}

int StrainTable :: get_strain_data_element(int strain, int i) {
  fred::Spin_Lock lock( mutex );
  //if(strain >= strains.size()) return -1;
  return strains[ strain ]->get_data_element(i);
}
    
void StrainTable :: printStrain(int strain_id, stringstream &out)
{
  if(strain_id >= strains.size()) return;
  strains[ strain_id ]->print_alternate(out);
}

std::string StrainTable::get_strain_data_string( int strain_id ) {
  return strains[ strain_id ]->to_string();
}
