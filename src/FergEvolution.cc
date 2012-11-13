/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

#include <map>
#include <cmath>
#include <fstream>
#include <cfloat>

#include "FergEvolution.h"
#include "MSEvolution.h"
#include "Random.h"
#include "Evolution.h"
#include "Infection.h"
#include "Trajectory.h"
#include "Global.h"
#include "IntraHost.h"
#include "Antiviral.h"
#include "Health.h"
#include "Person.h"
#include "Piecewise_Linear.h"
#include "Past_Infection.h"
#include "Transmission.h"
#include "Strain.h"
#include "Params.h"
#include "Utils.h"
#include "Population.h"
#include "Geo_Utils.h"
#include "Place.h"
#include "Large_Grid.h"
#include "Large_Cell.h"

using namespace std;

void FergEvolution :: setup( Disease * disease ) {

  MSEvolution::setup( disease );
  char numAA_file_name[ MAX_PARAM_SIZE ];
  strcpy( numAA_file_name, "$FRED_HOME/input_files/evolution/numAA.txt" );
  ifstream aafile;
  Utils::get_fred_file_name( numAA_file_name );
  aafile.open( numAA_file_name );
  if ( !( aafile.good() ) ) {
    Utils::fred_abort( "The file $FRED_HOME/input_files/evolution/numAA.txt (required by FergEvolution::setup) is non-existent, corrupted, empty, or otherwise not 'good()'... " );
  }
  while( !aafile.eof() ){
    int n;
    aafile >> n;
    aafile >> aminoAcids[ n ];
  }
  aafile.close();
  // get mutation probability (nucleotide)
  Params::get_param((char *) "mutation_rate", &delta);
  int num_amino_acids;
  Params::get_param((char *) "num_amino_acids", &num_amino_acids );
  num_ntides = num_amino_acids * 3;
  // setup cdf for mutation probability
  mutation_cdf.clear();
  // pass mutation cdf in by reference
  build_binomial_cdf( delta, num_ntides, mutation_cdf );

  for ( int n = 0; n < mutation_cdf.size(); ++n ) {
    std::cout << "mutation cdf " << mutation_cdf[ n ] << std::endl;
  }

  base_strain = 0;
  last_strain = base_strain;
  disease->add_root_strain( num_amino_acids ); 
  reignition_threshold = 1;

  vector< int > strains;
  strains.push_back( 0 );
  for ( int p = 0; p < Global::Pop.size(); ++p ) {
    Person * person = Global::Pop.get_person_by_index( p );
    double age = person->get_real_age();
    if ( age > 2 && RANDOM() < 0.25 ) {
      int recovery_date = 0 - IRAND( 0, 365 );
      int age_at_exp = age - ( (double) recovery_date / 365.0 );
      person->add_past_infection( strains, recovery_date, age_at_exp, disease );
    }
  }   
}

void FergEvolution::initialize_reporting_grid( Large_Grid * _grid ) {
  grid = _grid;
  num_clusters = grid->get_rows() * grid->get_cols();

  report = State< FergEvolution_Report >( fred::omp_get_max_threads() );
  for ( int t = 0; t < report.size(); ++t ) {
    report( t ).init( num_clusters );
  }
  report( 0 ).report_root_strain();
  weekly_report = NULL;

  initialize_reignitors( num_clusters );
}


FergEvolution::~FergEvolution() {
  clear_reignitors();
}

double FergEvolution::antigenic_distance( int strain1, int strain2 ) {
  double distance = 0.0;
  const Strain_Data & s1d = disease->get_strain_data( strain1 );
  const Strain_Data & s2d = disease->get_strain_data( strain2 );
  assert( s1d.size() == s2d.size() );
  for ( int i = 0; i < s1d.size(); ++i ){  
    if ( aminoAcids[ s1d[ i ] ] != aminoAcids[ s2d[ i ] ] ) {
      distance += 1;
    }
  }
  return distance;
}

double FergEvolution::genetic_distance( int strain1, int strain2 ) {
  double distance = 0.0;
  int n = disease->get_num_strain_data_elements( strain1 );
  for ( int i = 0; i < n; ++i ) {
    int seq1 = disease->get_strain_data_element( strain1, i );
    int seq2 = disease->get_strain_data_element( strain2, i );
    if ( seq1 % 4 != seq2 % 4 ) distance += 1;
    seq1 /= 4;
    seq2 /= 4;
    if ( seq1 % 4 != seq2 % 4 ) {
      distance += 1;
    }
    seq1 /= 4;
    seq2 /= 4;
    if ( seq1 % 4 != seq2 % 4 ) {
      distance += 1;
    }
  }
  distance /= ( 3 * n );
  return distance;
}

int FergEvolution :: get_cluster( Person * pers ) {
  return get_cluster( pers->get_health()->get_infection( disease->get_id() ) );
}

int FergEvolution :: get_cluster( Infection * infection ) {
  Place * household = infection->get_host()->get_household();
  fred::geo lon = household->get_longitude();
  fred::geo lat = household->get_latitude();
  return grid->get_grid_cell( lat, lon )->get_id() ;
}

void FergEvolution::try_to_mutate( Infection * infection, Transmission & transmission ) {

  int exp = infection->get_exposure_date();
  Trajectory * trajectory = infection->get_trajectory();
  int duration = trajectory->get_duration();
  vector< int > strains;
  infection->get_strains( strains );
  // only support single infections for now
  assert( strains.size() == 1 );
  // get the geographic cluster where this infection occurred
  int cell_id = get_cluster( infection );
  bool mutated = false;

  for ( int d = 0; d < duration; d++ ){
    for ( int k = 0; k < strains.size(); ++k ) {
      int parent_strain_id = strains[ k ];
      int child_strain_id = parent_strain_id;

      mutated = false;

      // find out how many mutations we'll have
      int n_mutations = draw_from_cdf_vector( mutation_cdf );
      // if zero, do nothing, otherwise pick which nucleotides will change
      // and report the mutation to the database
      if ( n_mutations > 0 ) {
        int mutated_nucleotides[ n_mutations ];
        if ( n_mutations == 1 ) {
          mutated_nucleotides[ 0 ] = IRAND( 0, num_ntides - 1 );
        }
        else {
          sample_range_without_replacement( num_ntides, n_mutations, mutated_nucleotides );
        }

        const Strain & parent_strain = disease->get_strain( parent_strain_id ); 
        const Strain_Data & parent_strain_data = disease->get_strain_data( parent_strain_id ); 
        // we know that we're going to have a mutation, so create the child strain.  If the mutation already
        // exists in the strain table we'll delete it later on
        Strain * new_strain = new Strain( parent_strain );
        Strain_Data & new_data = new_strain->get_data();

        for ( int i = 0; i < n_mutations; ++i ) {
          int aa_index = mutated_nucleotides[ i ] / 3;

          int nt_index_in_codon = 2 * ( mutated_nucleotides[ i ] % 3 );
          
          int bitmask = int( 3 ) << nt_index_in_codon; 
          new_data[ aa_index ] = 
            ( ( IRAND(0,63) ^ new_data[ aa_index ] ) & bitmask ) | ( parent_strain_data[ aa_index ] & (~bitmask) );
        }
     
        bool novel = false;
        { // <--------------------------------------------------- Mutex for add strain
          fred::Scoped_Lock lock( mutex );
          child_strain_id = disease->add_strain( new_strain, disease->get_transmissibility( parent_strain_id ), parent_strain_id );
          if ( child_strain_id > last_strain ) {
            novel = true;
            last_strain = child_strain_id;
          }
          else {
            delete new_strain;
          }
          // update the trajectory
          trajectory->mutate( parent_strain_id, child_strain_id, d ); 
          trajectory->mutate( parent_strain_id, child_strain_id, 0 ); // TODO wtf is this???
          FRED_VERBOSE( 1, "Evolution created a mutated strain! %5d   --> %5d %s\n",
              parent_strain_id, child_strain_id, novel ? " ...novel!" : "" );
        } // <--------------------------------------------------- Mutex for add strain
        // New strain created
        if ( novel ) {
          int pid = -1;
          int date = -1;
          
          if ( transmission.get_infected_place() ) {
            pid = transmission.get_infected_place()->get_id();
            date = transmission.get_exposure_date();
          }
          int infectee_id = infection->get_host()->get_id();
          int infector_id = -1;
          
          if ( transmission.get_infector()) {
            infector_id = transmission.get_infector()->get_id();
          }
        }
        // <----------------------- report mutation to database
        report().report_mutation( cell_id, child_strain_id, parent_strain_id, novel );
      }
      // <------------------------ report incidence to database (lockless, thread-safe)
      report().report_incidence( cell_id, child_strain_id ); 
      int exposed_duration = infection->get_infectious_date() - infection->get_exposure_date();
      int infectious_duration = infection->get_recovery_date() - infection->get_infectious_date();
      report().report_exposure( cell_id, child_strain_id, exposed_duration );
      report().report_prevalence( cell_id, child_strain_id, exposed_duration, infectious_duration );
      // update reignitor for this cell
      cache_reignitor( cell_id, child_strain_id );
    }
  }
}

void FergEvolution::add_failed_infection( Transmission & transmission, Person * infectee ) {
  Transmission::Loads * loads = transmission.get_initial_loads();
  vector< int > strains;
  Transmission::Loads::const_iterator it;
  for ( it = loads->begin(); it != loads->end(); ++it ) {
    strains.push_back( it->first );
  }
  int day = transmission.get_exposure_date();
  int recovery_date = day - 6;
  int age_at_exposure = infectee->get_age();
  infectee->add_past_infection( strains, recovery_date, age_at_exposure, disease );
}

double FergEvolution::get_prob_taking( Person * infectee, int new_strain, double quantity, int day ) {
  double prob_taking = MSEvolution::get_prob_taking( infectee, new_strain, quantity, day );
  return prob_taking;
}

Infection * FergEvolution::transmit( Infection * infection, Transmission & transmission, Person * infectee ) {
  infection = MSEvolution::transmit( infection, transmission, infectee );
  // if the infection failed, record as past_infection (stored in Health object of infectee)
  if ( infection == NULL ) {
    // immune boosting
    add_failed_infection( transmission, infectee );
    return infection;
  }
  // attempt to mutate this infection; this will also do reporting for this infection
  try_to_mutate( infection, transmission );
  // return the (possibly mutated) infection to Health
  return infection;
}

void FergEvolution::update( int day ) {

  // report cell stats (naive/total population per cell) every 30th day
  if ( day % 30 == 0 ) {
    Global::Large_Cells->report_grid_stats( day );
  }

  if ( report( 0 ).get_day() == FergEvolution_Report::window_size - 1 ) {
    weekly_report = new FergEvolution_Report( num_clusters );
    weekly_report->set_disease( disease );
    for ( int t = 0; t < report.size(); ++t ) {
      weekly_report->merge( report( t ) );
      report( t ).clear();
    }
    Global::db.enqueue_transaction( weekly_report );
  }
  else {
    for ( int t = 0; t < report.size(); ++t ) {
      report( t ).advance_day();
    }
  }

  // get total activity (prevalence from weekly_report).  If less than reignition threshold,
  // then reignite all cells.  THIS IS A PARALLEL METHOD
  int total_incidence = disease->get_epidemic()->get_incident_infections();
  if ( total_incidence < reignition_threshold ) {
    FRED_STATUS( 0, "Reignition! Total incidence = %d, reignition threshold = %d\n",
      total_incidence, reignition_threshold );
    reignite_all_cells( day );
  }

}

bool FergEvolution::reignite( Person * pers, int strain_id, int day ) {

  fred::Scoped_Lock lock( reignition_mutex );

  int disease_id = disease->get_id();
  Infection * curr_inf = pers->get_health()->get_infection( disease_id );
  
  // If the person is currently infected, don't reignite
  if ( curr_inf != NULL ) {
    return false;
  }
  // otherwise, make sure that they're susceptible
  else {
    pers->become_susceptible( disease );
  }

  Transmission transmission = Transmission( NULL, NULL, day );
  // force the transmission to succeed
  transmission.set_forcing( true );
  transmission.set_initial_loads( Evolution::get_primary_loads( day, strain_id ) );
  pers->become_exposed( this->disease, transmission );
  return true;
}

Transmission::Loads * FergEvolution::get_primary_loads(int day) {
  int strain = 0;
  return Evolution::get_primary_loads(day, strain);
}

void FergEvolution::terminate_person( Person * p ) { }

void FergEvolution::initialize_reignitors( int num_cells ) {
  reignitors = State< Reignitors_Array >( fred::omp_get_max_threads() );
  for ( int t = 0; t < fred::omp_get_max_threads(); ++t ) {
    reignitors( t ) = new int [ num_cells ];
    for ( int c = 0; c < num_cells; ++c ) {
      reignitors( t )[ c ] = -1;
    }
  }
}

void FergEvolution::cache_reignitor( int cell, int strain_id ) {
  FRED_STATUS( 4,
      "Caching strain for later reignition.  Thread = %d, cell = %d, strain_id = %d\n",
      fred::omp_get_thread_num(), cell, strain_id );
  reignitors()[ cell ] = strain_id;
}

void FergEvolution::clear_reignitors() {
  for ( int t = 0; t < fred::omp_get_max_threads(); ++t ) {
    delete[] reignitors( t );
  }
}

void FergEvolution::reignite_all_cells( int day ) {
  std::cout << "<------------------------------------------------------------------------------------ reginite all cells\n";
  double selection_probability = 1.0 / double( fred::omp_get_max_threads() );
  #pragma omp parallel for
  for ( int cell_id = 0; cell_id < num_clusters; ++cell_id ) {
    int strain = reignitors()[ cell_id ];
    if ( strain >= 0 && RANDOM() < selection_probability ) { 
      Large_Cell * cell = Global::Large_Cells->get_grid_cell_from_id( cell_id );
      FRED_STATUS( 0, "Reigniting person in cell %d with strain %d\n", cell_id, reignitors()[ cell_id ] );
      Person * person = cell->select_random_person();
      reignite( person, strain, day );
    }
  }
}
