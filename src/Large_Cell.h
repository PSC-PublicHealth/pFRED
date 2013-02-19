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
// File: Large_Cell.h
//

#ifndef _FRED_LARGE_CELL_H
#define _FRED_LARGE_CELL_H

#include <vector>

#include "Person.h"
#include "Abstract_Cell.h"
#include "Global.h"
#include "Utils.h"

#include "DB.h"

class Large_Grid;


struct Cell_Report : public Transaction {

  int day, cell_id, disease_id, naive, total;
  Cell_Report( int _day, int _cell_id, int _disease_id ) :
               day( _day ), cell_id( _cell_id ), disease_id( _disease_id ) {
    naive = 0;
    total = 0; 
  }

  void collect( std::vector< Person * > & person ) {
    std::vector< Person * >::iterator iter = person.begin();
    while ( iter != person.end() ) {
      Person & p = *(*iter); 
      if ( !( ( p.is_infectious( disease_id ) )
          && ( p.get_exposure_date( disease_id ) >= 0 )
          && ( p.get_num_past_infections( disease_id ) > 0 ) ) ) { ++naive; }
      ++total;
      ++iter;
    }
  }

 const char * initialize( int statement_number ) {
    if ( statement_number < n_stmts ) {
      if ( statement_number == 0 ) {
        return "create table if not exists cell_stats \
            ( day integer, cell integer, naive integer, total integer );";
      }
    }
    else {
      Utils::fred_abort( "Index of requested statement is out of range!", "" );
      return NULL;
    }
  }

  void prepare() {
    // statement number; reused for each statement's creation
    int S = 0;
    // define the statement and value variables
    n_stmts = 1;
    // new array of strings to hold statements
    statements = new std::string[ n_stmts ];
    // array giving the number of values expected for each statement
    n_values = new int[ n_stmts ];
    // array of vectors of string arrays  
    values = new std::vector< string * >[ n_stmts ];
    // <------------------------------------------------------------------------------------- prepare first statement
    S = 0;
    statements[ S ] = std::string( "insert into cell_stats values ($DAY,$CELL,$NAIVE,$TOTAL);" );
    n_values[ S ] = 4;
    std::string * values_array = new std::string[ n_values[ S ] ]; 
    std::stringstream ss;
    ss << day;
    values_array[ 0 ] = ss.str(); ss.str("");
    ss << cell_id;
    values_array[ 1 ] = ss.str(); ss.str("");
    ss << naive;
    values_array[ 2 ] = ss.str(); ss.str("");
    ss << total;
    values_array[ 3 ] = ss.str(); ss.str("");
    values[ S ].push_back( values_array );
  }
};


class Large_Cell : public Abstract_Cell {
public:
  Large_Cell(Large_Grid * grd, int i, int j);
  Large_Cell() {}
  ~Large_Cell() {}
  void setup(Large_Grid * grd, int i, int j);
  void quality_control();
  double distance_to_grid_cell(Large_Cell *grid_cell2);
  void add_person( Person * p ) {
    // <-------------------------------------------------------------- Mutex
    fred::Scoped_Lock lock(mutex);
    person.push_back( p );
    ++demes[ p->get_deme_id() ];
    ++popsize;
  }
  int get_popsize() { return popsize; }
  Person * select_random_person();
  void set_max_popsize(int n);
  int get_max_popsize() { return max_popsize; }
  double get_pop_density() { return pop_density; }
  void unenroll(Person *per);
  void add_workplace(Place *workplace);
  Place *get_workplace_near_to_school(Place *school);
  Place * get_closest_workplace(double x, double y, int min_size, int max_size, double * min_dist);

  int get_id() { return id; }

  Transaction * collect_cell_stats( int day, int disease_id );

  unsigned char get_deme_id();

protected:
  fred::Mutex mutex;
  Large_Grid * grid;
  int popsize;
  vector <Person *> person;
  int max_popsize;
  double pop_density;
  int id;
  static int next_cell_id;
  vector <Place *> workplaces;
  std::map< unsigned char, int > demes;
};

#endif // _FRED_LARGE_CELL_H
