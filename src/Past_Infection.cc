/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

#include "Past_Infection.h"
#include "Disease.h"
#include "Strain.h"
#include "Person.h"

using namespace std;

Past_Infection :: Past_Infection() { }

Past_Infection::Past_Infection( Infection * infection ) {
  infection->get_strains( strains );
  assert( strains.size() > 0 );
  recovery_date = infection->get_recovery_date();
  age_at_exposure = infection->get_age_at_exposure();
  if ( Global::Report_Immunity ) {
    report();
  }
}

Past_Infection::Past_Infection( vector< int > & _strains, int _recovery_date, int _age_at_exposure ) {
  strains = _strains;
  recovery_date = _recovery_date;
  age_at_exposure = _age_at_exposure;
  if ( Global::Report_Immunity ) {
    report();
  }
}

void Past_Infection::get_strains( vector< int > & strains ) {
  strains = this->strains;
}

void Past_Infection::report () {
  fprintf( Global::Immunityfp, "%d %d %zu ",
      recovery_date, age_at_exposure, strains.size() );
  for( int i = 0; i < strains.size(); i++ ){
    fprintf( Global::Immunityfp, "%d ", strains[ i ] );
  }
  fprintf(Global::Immunityfp, "\n");
}

string Past_Infection :: format_header() {
  return "# person_id disease_id recovery_date age_at_exposure num_strains [strains]+ \n";
}
