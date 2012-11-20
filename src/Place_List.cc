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
// File: Place_List.cc
//

#include <math.h>
#include <set>
#include <new>
#include <iostream>

#include "Place_List.h"
#include "Population.h"
#include "Disease.h"
#include "Global.h"
#include "Place.h"
#include "School.h"
#include "Classroom.h"
#include "Workplace.h"
#include "Office.h"
#include "Neighborhood.h"
#include "Household.h"
#include "Hospital.h"
#include "Params.h"
#include "Person.h"
#include "Grid.h"
#include "Large_Grid.h"
#include "Large_Cell.h"
#include "Small_Grid.h"
#include "Geo_Utils.h"
#include "Travel.h"
#include "Seasonality.h"
#include "Random.h"
#include "Utils.h"

// Place_List::quality_control implementation is very large,
// include from separate .cc file:
#include "Place_List_Quality_Control.cc"

Place_List::~Place_List() {
  delete_place_label_map();
}

void Place_List::get_parameters() {
}

void Place_List::read_places() {
  char s[80];
  char line_str[1024];
  char * line = line_str;
  char place_type;
  fred::geo lon, lat;
  fred::geo min_lat, max_lat, min_lon, max_lon;
  char location_file[256];
  FILE *fp = NULL;

  // vector to hold init data
  std::vector< Place_Init_Data > pidv;

  // vectors to hold household income
  std::vector< string > household_labels;
  std::vector< int > household_incomes;

  FRED_STATUS(0, "read places entered\n", "");

  // to compute the region's bounding box
  min_lat = min_lon = 999;
  max_lat = max_lon = -999;

  // initialize counts to zero
  place_type_counts[ HOUSEHOLD ] = 0;       // 'H'
  place_type_counts[ SCHOOL ] = 0;          // 'S'
  place_type_counts[ WORKPLACE ] = 0;       // 'W'
  place_type_counts[ HOSPITAL ] = 0;        // 'M'
  place_type_counts[ NEIGHBORHOOD ] = 0;    // 'N'
  place_type_counts[ CLASSROOM ] = 0;       // 'C'
  place_type_counts[ OFFICE ] = 0;          // 'O'
  place_type_counts[ COMMUNITY ] = 0;       // 'X'

  // read ver 2.0 synthetic population files
  char newline[1024];
  char hh_id[32];
  char serialno[32];
  char stcotrbg[32];
  char hh_race[32];
  char hh_income[32];
  char hh_size[32];
  char hh_age[32];
  char latitude[32];
  char longitude[32];
  int income;
  char workplace_id[32];
  char num_workers_assigned[32];
  char school_id[32];
  char name[64];
  char stabbr[32];
  char address[64];
  char city[32];
  char county[32];
  char zip[32];
  char zip4[32];
  char nces_id[32];
  char total[32];
  char prek[32];
  char kinder[32];
  char gr01_gr12[32];
  char ungraded[32];
  char source[32];
  char stco[32];

  household_labels.clear();
  household_incomes.clear();

  // read household locations
  sprintf(location_file, "%s/%s/%s_synth_households.txt", 
	  Global::Synthetic_population_directory,
	  Global::Synthetic_population_id,
	  Global::Synthetic_population_id);
  fp = Utils::fred_open_file(location_file);
  while (fgets(line, 1024, fp) != NULL) {
    // printf("%s%s",line,newline); fflush(stdout); exit(0);
    Utils::get_next_token(hh_id, &line);
    // skip header line
    if (strcmp(hh_id, "hh_id") == 0) continue;
    Utils::get_next_token(serialno, &line);
    Utils::get_next_token(stcotrbg, &line);
    Utils::get_next_token(hh_race, &line);
    Utils::get_next_token(hh_income, &line);
    Utils::get_next_token(hh_size, &line);
    Utils::get_next_token(hh_age, &line);
    Utils::get_next_token(latitude, &line);
    Utils::get_next_token(longitude, &line);

    place_type = 'H';
    sprintf(s, "%c%s", place_type, hh_id);
    sscanf(latitude, "%f", &lat);
    sscanf(longitude, "%f", &lon);

    pidv.push_back( Place_Init_Data( s, place_type, lat, lon ) );
    ++( place_type_counts[ place_type ] );
    // printf ("%s %c %f %f\n", s, place_type,lat,lon); fflush(stdout);

    // save household incomes for later processing
    sscanf(hh_income, "%d", &income);
    household_incomes.push_back(income);
    household_labels.push_back(string(s));
    line = line_str;
  }
  fclose(fp);

  // read workplace locations
  sprintf(location_file, "%s/%s/%s_workplaces.txt", 
	  Global::Synthetic_population_directory,
	  Global::Synthetic_population_id,
	  Global::Synthetic_population_id);
  fp = Utils::fred_open_file(location_file);
  while (fgets(line, 255, fp) != NULL) {
    Utils::get_next_token(workplace_id, &line);
    // skip header line
    if (strcmp(workplace_id, "workplace_id") == 0) continue;
    Utils::get_next_token(num_workers_assigned, &line);
    Utils::get_next_token(latitude, &line);
    Utils::get_next_token(longitude, &line);

    place_type = 'W';
    sprintf(s, "%c%s", place_type, workplace_id);
    sscanf(latitude, "%f", &lat);
    sscanf(longitude, "%f", &lon);

    pidv.push_back( Place_Init_Data( s, place_type, lat, lon ) );
    ++( place_type_counts[ place_type ] );
    // printf ("%s %c %f %f\n", s, place_type,lat,lon); fflush(stdout);
    line = line_str;
  }
  fclose(fp);

  // read school locations
  sprintf(location_file, "%s/%s/%s_schools.txt", 
	  Global::Synthetic_population_directory,
	  Global::Synthetic_population_id,
	  Global::Synthetic_population_id);
  fp = Utils::fred_open_file(location_file);
  while (fgets(line, 255, fp) != NULL) {
    Utils::get_next_token(school_id, &line);
    // skip header line
    if (strcmp(school_id, "school_id") == 0) continue;
    Utils::get_next_token(name, &line);
    Utils::get_next_token(stabbr, &line);
    Utils::get_next_token(address, &line);
    Utils::get_next_token(city, &line);
    Utils::get_next_token(county, &line);
    Utils::get_next_token(zip, &line);
    Utils::get_next_token(zip4, &line);
    Utils::get_next_token(nces_id, &line);
    Utils::get_next_token(total, &line);
    Utils::get_next_token(prek, &line);
    Utils::get_next_token(kinder, &line);
    Utils::get_next_token(gr01_gr12, &line);
    Utils::get_next_token(ungraded, &line);
    Utils::get_next_token(latitude, &line);
    Utils::get_next_token(longitude, &line);
    Utils::get_next_token(source, &line);
    Utils::get_next_token(stco, &line);

    place_type = 'S';
    sprintf(s, "%c%s", place_type, school_id);
    sscanf(latitude, "%f", &lat);
    sscanf(longitude, "%f", &lon);

    pidv.push_back( Place_Init_Data( s, place_type, lat, lon ) );
    ++( place_type_counts[ place_type ] );

    FRED_VERBOSE(0, "READ_SCHOOL: %s %c %f %f name |%s|\n", s, place_type,lat,lon,name);
    line = line_str;
  }
  fclose(fp);

  // sort the place init data by type, location ( using Place_Init_Data::operator< )
  std::sort( pidv.begin(), pidv.end() );

  // HOUSEHOLD in-place allocator
  Place::Allocator< Household > household_allocator;
  household_allocator.reserve( place_type_counts[ HOUSEHOLD ] );
  // SCHOOL in-place allocator
  Place::Allocator< School > school_allocator;
  school_allocator.reserve( place_type_counts[ SCHOOL ] );
  // WORKPLACE in-place allocator
  Place::Allocator< Workplace > workplace_allocator;
  workplace_allocator.reserve( place_type_counts[ WORKPLACE ] );
  // HOSPITAL in-place allocator
  Place::Allocator< Hospital > hospital_allocator;
  hospital_allocator.reserve( place_type_counts[ HOSPITAL ] );
  
  // fred-specific place types initialized elsewhere (setup_offices, setup_classrooms)

  // more temporaries
  Place * place = NULL;
  Place * container = NULL;

  // loop through sorted init data and create objects using Place_Allocator
  for ( int i = 0; i < pidv.size(); ++i ) {
      strcpy( s, pidv[ i ].s );
      place_type = pidv[ i ].place_type;
      lon = pidv[ i ].lon;
      lat = pidv[ i ].lat;

      if (place_type == HOUSEHOLD && lat != 0.0) {
        if (lat < min_lat) min_lat = lat;
        if (max_lat < lat) max_lat = lat;
      }
      if (place_type == HOUSEHOLD && lon != 0.0) {
        if (lon < min_lon) min_lon = lon;
        if (max_lon < lon) max_lon = lon;
      }
      if (place_type == HOUSEHOLD) {
        place = new ( household_allocator.get_free() )
          Household( s, lon, lat, container, &Global::Pop );
      }
      else if (place_type == SCHOOL) {
        place = new ( school_allocator.get_free() )
          School( s, lon, lat, container, &Global::Pop );
      }
      else if (place_type == WORKPLACE) {
        place = new ( workplace_allocator.get_free() )
          Workplace( s, lon, lat, container, &Global::Pop );
      }
      else if (place_type == HOSPITAL) {
        place = new ( hospital_allocator.get_free() )
          Hospital( s, lon, lat, container, &Global::Pop );
      }
      else {
        Utils::fred_abort("Help! bad place_type %c\n", place_type); 
      }
      
      if (place == NULL) {
        Utils::fred_abort( "Help! allocation failure for the %dth entry in location file (s=%s, type=%c)\n",
            i,s,place_type ); 
      }

      place = NULL;
  }

  // since everything was allocated in contiguous blocks, we can use pointer arithmetic
  // call to add_preallocated_places also ensures that all allocations were used for
  // successful additions to the place list
  add_preallocated_places< Household >( HOUSEHOLD, household_allocator ); 
  add_preallocated_places< School    >( SCHOOL,    school_allocator    ); 
  add_preallocated_places< Workplace >( WORKPLACE, workplace_allocator ); 
  add_preallocated_places< Hospital  >( HOSPITAL,  hospital_allocator  ); 

  // set the household income
  if (strcmp(Global::Synthetic_population_id,"none")!=0) {
    for (int i = 0; i < household_incomes.size(); i++) {
      Household * h = (Household *) get_place_from_label((char*) household_labels[i].c_str());
      h->set_household_income(household_incomes[i]);
      FRED_VERBOSE( 1, "INC: %s %c %f %f %d\n", h->get_label(), h->get_type(),
		    h->get_latitude(), h->get_longitude(), h->get_household_income());
    }
  }

  FRED_STATUS(0, "finished reading %d locations, now creating additional FRED locations\n", next_place_id);

  if (Global::Use_Mean_Latitude) {
    // Make projection based on the location file.
    fred::geo mean_lat = (min_lat + max_lat) / 2.0;
    Geo_Utils::set_km_per_degree(mean_lat);
    printf("min_lat: %f  max_lat: %f  mean_lat: %f\n", min_lat, max_lat, mean_lat);
  }
  else {
    // DEFAULT: Use mean US latitude (see Geo_Utils.cc)
    printf("min_lat: %f  max_lat: %f\n", min_lat, max_lat);
  }

  // create geographical grids
  Global::Large_Cells = new Large_Grid(min_lon, min_lat, max_lon, max_lat);

  // Initialize global seasonality object
  if (Global::Enable_Seasonality) {
    Global::Clim = new Seasonality(Global::Large_Cells);
  }

  // Grid/Cells contain neighborhoods
  Global::Cells = new Grid(Global::Large_Cells);

  int number_places = (int) places.size();
  for (int p = 0; p < number_places; p++) {

    // add households to the 1km Cell
    if (places[p]->get_type() == HOUSEHOLD) {
      Place *place = places[p];
      int row = Global::Cells->get_row(place->get_latitude());
      int col = Global::Cells->get_col(place->get_longitude());
      Cell * grid_cell = Global::Cells->get_grid_cell(row,col);

      FRED_CONDITIONAL_VERBOSE( 0, grid_cell == NULL,
          "Help: household %d has bad grid_cell,  lat = %f  lon = %f\n",
          place->get_id(),lat,lon); 

      assert(grid_cell != NULL);

      grid_cell->add_household(place);
      place->set_grid_cell(grid_cell);
    }
  }

  int number_of_neighborhoods = Global::Cells->get_number_of_neighborhoods();

  // create allocator for neighborhoods
  Place::Allocator< Neighborhood > neighborhood_allocator;

  // reserve enough space for all neighborhoods
  neighborhood_allocator.reserve( number_of_neighborhoods );
  FRED_STATUS( 0, "Allocated space for %7d neighborhoods\n", number_of_neighborhoods );

  // pass allocator to Grid::setup (which then passes to Cell::make_neighborhood)
  Global::Cells->setup( neighborhood_allocator );

  // add Neighborhoods in one contiguous block
  add_preallocated_places< Neighborhood >( NEIGHBORHOOD, neighborhood_allocator );

  if (Global::Enable_Small_Grid)
    Global::Small_Cells = new Small_Grid(Global::Large_Cells);

  number_places = (int) places.size();
  for (int p = 0; p < number_places; p++) {

    // add workplaces to the 20km Large_Cell (needed for teacher assignments to schools)
    if (places[p]->get_type() == WORKPLACE) {
      Place *place = places[p];
      int row = Global::Large_Cells->get_row(place->get_latitude());
      int col = Global::Large_Cells->get_col(place->get_longitude());
      Large_Cell * grid_cell = Global::Large_Cells->get_grid_cell(row,col);
      if (grid_cell != NULL) {
	grid_cell->add_workplace(place);
      }
    }
  }

  FRED_STATUS(0, "read places finished: Places = %d\n", (int) places.size());
}

void Place_List::prepare() {

  FRED_STATUS(0, "prepare places entered\n","");

  int number_places = places.size();
  for (int p = 0; p < number_places; p++) {
    places[p]->prepare();
  }

  FRED_STATUS( 0, "deleting place_label_map\n","" );
  delete_place_label_map();

  FRED_STATUS(0, "prepare places finished\n","");

}

void Place_List::update(int day) {

  FRED_STATUS(1, "update places entered\n","");

  if (Global::Enable_Seasonality) {
    Global::Clim->update(day);
  }
  int number_places = places.size();
  #pragma omp parallel for
  for ( int p = 0; p < number_places; ++p ) {
    places[ p ]->update( day );
  }

  FRED_STATUS(1, "update places finished\n", "");
}

Place * Place_List::get_place_from_label(char *s) {
  if (strcmp(s, "-1") == 0) return NULL;
  string str;
  str.assign(s);
  if (place_label_map->find(s) != place_label_map->end())
    return places[ (*place_label_map)[s] ];
  else {
    FRED_VERBOSE(1, "Help!  can't find place with label = %s\n", s);
    return NULL;
  }
}

int Place_List::add_place( Place * p ) {
  int index = -1;
  // p->print(0);
  // assert(place_map.find(p->get_d()) == place_map.end());
  //
  FRED_CONDITIONAL_WARNING( p->get_id() != -1,
      "Place id (%d) was overwritten!", p->get_id() ); 
  assert( p->get_id() == -1 );

    string str;
    str.assign( p->get_label() );

    if ( place_label_map->find(str) == place_label_map->end() ) {
 
      p->set_id( get_new_place_id() );
     
      places.push_back(p);

      (*place_label_map)[ str ] = places.size() - 1;

      // printf("places now = %d\n", (int)(places.size())); fflush(stdout);
     
      // TODO workplaces vector won't be needed once all places stored and labeled in bloque
      if (Global::Enable_Local_Workplace_Assignment && p->is_workplace()) {
        workplaces.push_back(p);
      }

    }
    else {
      printf("Warning: duplicate place label found: ");
      p->print(0);
    }

  return index;
}

int Place_List::get_number_of_places( char place_type ) {
  assert( place_type_counts.find( place_type ) !=
          place_type_counts.end() );
  return place_type_counts[ place_type ];
}

void Place_List::setup_classrooms() {

  FRED_STATUS(0, "setup classrooms entered\n","");

  int number_classrooms = 0;
  int number_places = places.size();
  int number_schools = 0;

  //#pragma omp parallel for reduction(+:number_classrooms)
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == SCHOOL) {
      School * school = (School *) places[p];
      number_classrooms += school->get_number_of_rooms();
      ++( number_schools );
    }
  }

  Place::Allocator< Classroom > classroom_allocator;
  classroom_allocator.reserve( number_classrooms );

  FRED_STATUS( 0, "Allocating space for %d classrooms in %d schools (out of %d total places)\n",
      number_classrooms, number_schools, number_places );

  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == SCHOOL) {
      School * school = (School *) places[p];
      school->setup_classrooms( classroom_allocator );
    }
  }

  add_preallocated_places< Classroom >( CLASSROOM, classroom_allocator );

  FRED_STATUS(0, "setup classrooms finished\n","");
}

double distance_between_places(Place * p1, Place * p2) {
  return Geo_Utils::xy_distance(p1->get_latitude(), p1->get_longitude(),
				p2->get_latitude(), p2->get_longitude());
}

void Place_List::assign_teachers() {
  int number_places = places.size();
  printf("assign teachers entered. places = %d\n", number_places);
  fflush(stdout);
  for (int p = 0; p < number_places; p++) {
    Place *school = places[p];
    if (school->get_type() == SCHOOL) {
      fred::geo lat = school->get_latitude();
      fred::geo lon = school->get_longitude();
      double x = Geo_Utils::get_x(lon);
      double y = Geo_Utils::get_y(lat);
      FRED_VERBOSE(0,"School %s %f %f ", school->get_label(), x, y);

      // ignore school if it is outside the region (because we probably
      // do not have the teachers in the workforce)
      Large_Cell * large_cell = Global::Large_Cells->get_grid_cell(lat,lon);
      if (large_cell == NULL) {
	FRED_VERBOSE(0, "school OUTSIDE_REGION lat %f lon %f \n", lat, lon);
	continue;
      }
      
      Place * nearby_workplace = large_cell->get_workplace_near_to_school(school);
      if (nearby_workplace != NULL) {
	// make all the workers in selected workplace teachers at the nearby school
	nearby_workplace->turn_workers_into_teachers(school);
      }
      else {
	FRED_VERBOSE(0, "NO NEARBY_WORKPLACE FOUND for school at lat %f lon %f \n", lat, lon);
      }
    }
  }
  fflush(stdout);
}

void Place_List::setup_offices() {

  FRED_STATUS(0, "setup offices entered\n","");

  int number_offices = 0;
  int number_places = places.size();
 
  #pragma omp parallel for reduction(+:number_offices)
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == WORKPLACE) {
      Workplace * workplace = (Workplace *) places[p];
      number_offices += workplace->get_number_of_rooms();
    }
  }

  Place::Allocator< Office > office_allocator;
  office_allocator.reserve( number_offices );

  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == WORKPLACE) {
      Workplace * workplace = (Workplace *) places[p];
      workplace->setup_offices( office_allocator );
    }
  }
  // add offices in one contiguous block to Place_List
  add_preallocated_places< Office >( OFFICE, office_allocator );

  FRED_STATUS(0, "setup offices finished\n","");
}

Place * Place_List::get_random_workplace() {
  int size = workplaces.size();
  if (size > 0) {
    return workplaces[IRAND(0,size-1)];
  }
  else
    return NULL;
}

void Place_List::print_household_size_distribution(char * dir, char * date_string, int run) {
  FILE *fp;
  int count[11];
  double pct[11];
  char filename[256];
  sprintf(filename, "%s/household_size_dist_%s.%02d", dir, date_string, run);
  printf("print_household_size_dist entered, filename = %s\n", filename); fflush(stdout);
  for (int i = 0; i < 11; i++) {
    count[i] = 0;
  }
  int total = 0;
  int number_places = (int) places.size();
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == HOUSEHOLD) {
      int n = places[p]->get_size();
      if (n < 11) { count[n]++; }
      else { count[10]++; }
      total++;
    }
  }
  fp = fopen(filename, "w");
  for (int i = 0; i < 11; i++) {
    pct[i] = (100.0*count[i])/number_places;
    fprintf(fp, "size %d count %d pct %f\n", i*5, count[i], pct[i]);
  }
  fclose(fp);
}

void Place_List::end_of_run() {
  if (Global::Verbose > 1) {
    int number_places = places.size();
    for (int p = 0; p < number_places; p++) {
      Place *place = places[p];
      fprintf(Global::Statusfp,"PLACE REPORT: id %d type %c size %d days_inf %d attack_rate %5.2f\n",
          place->get_id(), place->get_type(), place->get_size(),
          place->get_days_infectious(), 100.0*place->get_attack_rate());
    }
  }
}


 void Place_List::delete_place_label_map() {
   if ( place_label_map ) {
     delete place_label_map;
     place_label_map = NULL;
   }
 }
