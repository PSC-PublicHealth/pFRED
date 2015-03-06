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
// File: Population.cc
//

#include <stdio.h>
#include <new>
#include <string>
#include <sstream>
#include <fstream>
#include <limits>
#include <set>


#include "Population.h"
#include "Params.h"
#include "Global.h"
#include "Place_List.h"
#include "Household.h"
#include "Disease.h"
#include "Person.h"
#include "Demographics.h"
#include "Manager.h"
#include "AV_Manager.h"
#include "Vaccine_Manager.h"
#include "Age_Map.h"
#include "Random.h"
#include "Date.h"
#include "Travel.h"
#include "Utils.h"
#include "Past_Infection.h"
#include "Evolution.h"
#include "Activities.h"
#include "Behavior.h"
#include "Tracker.h"
#include "Vaccine_Health.h"
#include "AV_Health.h"


#include <snappy.h>
using namespace std; 

// used for reporting
int age_count_male[Demographics::MAX_AGE + 1];
int age_count_female[Demographics::MAX_AGE + 1];
int birth_count[Demographics::MAX_AGE + 1];
int death_count_male[Demographics::MAX_AGE + 1];
int death_count_female[Demographics::MAX_AGE + 1];

char Population::pop_outfile[FRED_STRING_SIZE];
char Population::output_population_date_match[FRED_STRING_SIZE];
int  Population::output_population = 0;
bool Population::is_initialized = false;
int  Population::next_id = 0;

Population::Population() {

  clear_static_arrays();
  birthday_map.clear();
  pop_size = 0;
  disease = NULL;
  av_manager = NULL;
  vacc_manager = NULL;
  mutation_prob = NULL;

  for ( int i = 0; i < 367; ++i ) {
    birthday_vecs[ i ].clear();
  }
}

void Population::initialize_masks() {
  // can't do this in the constructor because the Global:: variables aren't yet
  // available when the Global::Pop is defined
  blq.add_mask( fred::Infectious );
  blq.add_mask( fred::Susceptible );
  blq.add_mask( fred::Update_Deaths );
  blq.add_mask( fred::Update_Births );
  blq.add_mask( fred::Update_Health );
}


// index and id are not the same thing!

Person * Population::get_person_by_index( int _index ) {
  return blq.get_item_pointer_by_index( _index );
}

//Person * Population::get_person_by_id( int _id ) {
//  return blq.get_item_pointer_by_index( id_to_index[ _id ] ); 
//}

Population::~Population() {
  // free all memory
  pop_size = 0;
  if ( disease != NULL ) delete [] disease;
  if ( vacc_manager != NULL ) delete vacc_manager;
  if ( av_manager != NULL ) delete av_manager;
  if ( mutation_prob != NULL ) delete [] mutation_prob;
}

void Population::get_parameters() {

  int num_mutation_params =
    Params::get_param_matrix((char *) "mutation_prob", &mutation_prob);
  if (num_mutation_params != Global::Diseases) {
    fprintf(Global::Statusfp,
        "Improper mutation matrix: expected square matrix of %i rows, found %i",
        Global::Diseases, num_mutation_params);
    exit(1);
  }

  if (Global::Verbose > 1) {
    printf("\nmutation_prob:\n");
    for (int i  = 0; i < Global::Diseases; i++)  {
      for (int j  = 0; j < Global::Diseases; j++) {
        printf("%f ", mutation_prob[i][j]);
      }
      printf("\n");
    }
  }

  // Only do this one time
  if(!Population::is_initialized) {
    Params::get_param_from_string("output_population", &Population::output_population);
    if(Population::output_population > 0) {
      Params::get_param_from_string("pop_outfile", Population::pop_outfile);
      Params::get_param_from_string("output_population_date_match", Population::output_population_date_match);
    }
    Population::is_initialized = true;
  }
}

/*
 * All Persons in the population must have been created using add_person
 */
Person * Population::add_person( int age, char sex, int race, int rel, Place *house,
    Place *school, Place *work, int day, bool today_is_birthday ) {

  fred::Scoped_Lock lock( add_person_mutex );

  int id = Population::next_id++; 
  int idx = blq.get_free_index();

  Person * person = blq.get_free_pointer( idx );

  // mark valid before adding person so that mask operations will be
  // available in the constructor (of Person and all ancillary objects)
  blq.mark_valid_by_index( idx ); 

  new( person ) Person();
  
  person->setup( idx, id, age, sex, race, rel, 
      house, school, work, day, today_is_birthday );

  //assert( id_to_index.find( id ) == id_to_index.end() );
  //id_to_index[ id ] = idx;

  assert( (unsigned) pop_size == blq.size() - 1 );

  pop_size = blq.size();

  if ( Global::Enable_Aging ) {
    Demographics * demographics = person->get_demographics();
    int pos = demographics->get_birth_day_of_year();
    //Check to see if the day of the year is after FEB 28
    if ( pos > 59 && !Date::is_leap_year( demographics->get_birth_year() ) ) {
      pos++;
    }
    birthday_vecs[ pos ].push_back( person );
    FRED_VERBOSE( 2,
        "Adding person %d to birthday vector for day = %d.\n ... birthday_vecs[ %d ].size() = %zu\n",
        id, pos, pos, birthday_vecs[ pos ].size() );
    birthday_map[ person ] = ( (int) birthday_vecs[ pos ].size() - 1 );
  }
  return person;
}

void Population::set_mask_by_index( fred::Pop_Masks mask, int person_index ) {
  // assert that the mask has in fact been added
  blq.set_mask_by_index( mask, person_index );
}

void Population::clear_mask_by_index( fred::Pop_Masks mask, int person_index ) {
  // assert that the mask has in fact been added
  blq.clear_mask_by_index( mask, person_index );
}

void Population::delete_person(Person * person) {
  FRED_VERBOSE(1,"DELETING PERSON: %d ...\n", person->get_id());

  person->terminate();
  FRED_VERBOSE(1,"DELETED PERSON: %d\n", person->get_id());

  for (int d = 0; d < Global::Diseases; d++) {
    disease[d].get_evolution()->terminate_person(person);
  }

  if ( Global::Enable_Travel ) {
    Travel::terminate_person(person);
  }

  //assert( id_to_index.find( person->get_id() ) != id_to_index.end() );
  //id_to_index.erase( person->get_id() );

  int idx = person->get_pop_index();
  assert( get_person_by_index( idx ) == person );
  // call Person's destructor directly!!!
  get_person_by_index( idx ) -> ~Person();
  blq.mark_invalid_by_index( person->get_pop_index() );

  pop_size--;

  if ((unsigned) pop_size != blq.size()) {
    FRED_VERBOSE(0,"pop_size = %d  blq.size() = %d\n",
        pop_size, (int) blq.size());
  }
  assert( (unsigned) pop_size == blq.size() );
}

void Population::prepare_to_die( int day, int person_index ) {
  Person * per = get_person_by_index( person_index );
  fred::Scoped_Lock lock( mutex );
  // add person to daily death_list
  death_list.push_back(per);
  report_death(day, per);
  // you'll be stone dead in a moment...
  per->die();
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "prepare to die: ");
    per->print(Global::Statusfp,0);
  }
}

void Population::prepare_to_give_birth( int day, int person_index ) {
  Person * per = get_person_by_index( person_index );
  fred::Scoped_Lock lock( mutex );
  // add person to daily maternity_list
  maternity_list.push_back(per);
  report_birth(day, per);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp,"prepare to give birth: ");
    per->print(Global::Statusfp,0);
  }
}

void Population::setup() {
  FRED_STATUS(0, "setup population entered\n","");

  disease = new Disease [Global::Diseases];
  for (int d = 0; d < Global::Diseases; d++) {
    disease[d].setup(d, this, mutation_prob[d]);
  }

  if ( Global::Enable_Vaccination ) {
    vacc_manager = new Vaccine_Manager(this);
  } else {
    vacc_manager = new Vaccine_Manager();
  }

  if ( Global::Enable_Antivirals ) {
    av_manager = new AV_Manager(this);
  } else {
    av_manager = new AV_Manager();
  }

  if (Global::Verbose > 1) av_manager->print();

  // TODO provide clear() method for bloque
  //id_to_index.clear();
  pop_size = 0;
  maternity_list.clear();
  death_list.clear();
  read_all_populations();

  // empty out the incremental list of Person's who have changed
  incremental_changes.clear();
  never_changed.clear();
  /*
     for (int p = 0; p < pop_size; p++){
     never_changed[pop[p]]=true; // add all agents to this list at start
     }
     */

  // TODO this assumes only one disease, need to rewrite to look at all diseases
  if ( Global::Verbose > 0 ) {
    int count = 0;
    for ( int p = 0; p < pop_size; p++ ){
      Disease * d = &disease[ 0 ];
      if ( blq.get_item_reference_by_index( p ).get_health()->is_immune(d) ) { count++; }
    }
    FRED_STATUS(0, "number of residually immune people = %d\n", count);
  }
  av_manager->reset();
  vacc_manager->reset();

  FRED_STATUS(0, "population setup finished\n","");
}

Person_Init_Data Population::get_person_init_data( char * line,
    const Place_List & places, bool is_group_quarters_population ) {

  char newline[1024];
  Utils::replace_csv_missing_data(newline, line, "-1");
  Utils::Tokens tokens = Utils::split_by_delim( newline, ',', false );
  const PopFileColIndex & col = get_pop_file_col_index( is_group_quarters_population );
  assert( int( tokens.size() ) == col.number_of_columns );
  // initialized with default values
  Person_Init_Data pid = Person_Init_Data();
  strcpy( pid.label, tokens[ col.p_id ] );
  // add type indicator to label for places
  if ( is_group_quarters_population ) {
    pid.in_grp_qrtrs = true;
    sscanf( tokens[ col.gq_type ], "%c", & pid.gq_type );
    // special formatting for GQ house and workplace labels
    if ( strcmp( tokens[ col.home_id ], "-1" ) ) {
      sprintf( pid.house_label, "H%s-%c", tokens[ col.home_id ],
          pid.gq_type, pid.gq_type );
    }
    if ( strcmp( tokens[ col.workplace_id ], "-1" ) ) {
      sprintf( pid.work_label, "W%s-%c", tokens[ col.workplace_id ],
          pid.gq_type, pid.gq_type );
    }
  }
  else {
    // columns not present in group quarters population
    sscanf( tokens[ col.relate ], "%d", & pid.relationship );
    sscanf( tokens[ col.race_str ], "%d", & pid.race );
    // schools only defined for synth_people
    if ( strcmp( tokens[ col.school_id ], "-1" ) ) {
      sprintf( pid.school_label, "S%s", tokens[ col.school_id ] );
    }
    // standard formatting for house and workplace labels
    if ( strcmp( tokens[ col.home_id ], "-1" ) ) {
      sprintf( pid.house_label, "H%s", tokens[ col.home_id ] );
    }
    if ( strcmp( tokens[ col.workplace_id ], "-1" ) ) {
      sprintf( pid.work_label, "W%s", tokens[ col.workplace_id ] );
    }
  }
  // age, sex same for synth_people and synth_gq_people
  sscanf( tokens[ col.age_str ], "%d", & pid.age );
  pid.sex = strcmp( tokens[ col.sex_str ], "1" )==0 ? 'M' : 'F';
  // set pointer to primary places in init data object
  pid.house = places.get_place_from_label( pid.house_label );
  pid.work = places.get_place_from_label( pid.work_label );
  pid.school = places.get_place_from_label( pid.school_label );
  // warn if we can't find workplace
  if ( strcmp( pid.work_label, "-1" ) != 0 && pid.work == NULL ) {
    FRED_VERBOSE( 2, "WARNING: person %s -- no workplace found for label = %s\n",
        pid.label, pid.work_label );
    if ( Global::Enable_Local_Workplace_Assignment ) {
      pid.work = Global::Places.get_random_workplace();
      FRED_CONDITIONAL_VERBOSE( 0, pid.work != NULL,
          "WARNING: person %s assigned to workplace %s\n",
          pid.label, pid.work->get_label() );
      FRED_CONDITIONAL_VERBOSE( 0, pid.work == NULL,
          "WARNING: no workplace available for person %s\n",
          pid.label );
    }
  }
  // warn if we can't find school.  No school for gq_people
  FRED_CONDITIONAL_VERBOSE( 0,
      (strcmp(pid.school_label,"-1")!=0 && pid.school == NULL),
      "WARNING: person %s -- no school found for label = %s\n",
      pid.label, pid.school_label);

  return pid;
}

void Population::parse_lines_from_stream( std::istream & stream,
    bool is_group_quarters_pop ) {

  // vector used for batch add of new persons
  std::vector< Person_Init_Data > pidv;

  while ( stream.good() ) {
    char line[FRED_STRING_SIZE];
    stream.getline( line, FRED_STRING_SIZE );
    // skip empty lines...
    if ( ( line[ 0 ] == '\0' ) || strncmp( line, "p_id", 4 ) == 0 ) continue;
    //printf("line: |%s|\n", line); fflush(stdout); // exit(0);
    const Person_Init_Data & pid = get_person_init_data( line, Global::Places,
        is_group_quarters_pop ); 
    // verbose printing of all person initialization data
    FRED_VERBOSE( 1, "%s\n", pid.to_string().c_str() );
    //skip header line
    if (strcmp(pid.label,"p_id")==0) continue;
    /*
    printf("|%s %d %c %d %s %s %s %d|\n", label, age, sex, race
        house_label, work_label, school_label, relationship);
    fflush(stdout);
    
    if (strcmp(work_label,"-1") && strcmp(school_label,"-1")) {
      printf("STUDENT-WORKER: %s %d %c %s %s\n", label, age, sex, work_label, school_label);
      fflush(stdout);
    }
    */
    if ( pid.house != NULL ) {
      // create a Person_Init_Data object
      pidv.push_back( pid );
    }
    else {
      // we need at least a household (homeless people not yet supported), so
      // skip this person
      FRED_VERBOSE( 0, "WARNING: skipping person %s -- %s %s\n",
          pid.label, "no household found for label =", pid.house_label );
    }
  } // <----- end while loop over stream

  // Iterate through vector of already parsed initialization data and
  // add to population bloque.  More efficient to do this in batches; also
  // preserves the (fine-grained) order in the population file.  Protect
  // with mutex so that we do this sequentially and avoid thrashing the 
  // scoped mutex in add_person.
  fred::Scoped_Lock lock( batch_add_person_mutex );
  std::vector< Person_Init_Data >::iterator it = pidv.begin();
  for ( ; it != pidv.end(); ++it ) {
    Person_Init_Data & pid = *it;
    // here the person is actually created and added to the population
    // The person's unique id is automatically assigned
    add_person( pid.age, pid.sex, pid.race, pid.relationship,
        pid.house, pid.school, pid.work, pid.day, pid.today_is_birthday);
  }
}

void Population::split_synthetic_populations_by_deme() {
  using namespace std;
  using namespace Utils;
  const char delim = ' ';

  FRED_STATUS( 0, "Validating synthetic population identifiers before reading.\n", "" );
  const char * pop_dir = Global::Synthetic_population_directory;
  FRED_STATUS( 0, "Using population directory: %s\n", pop_dir );
  FRED_STATUS( 0, "FRED defines a \"deme\" to be a local population %s\n%s%s\n",
      "of people whose households are contained in the same bounded geographic region.",
      "No Synthetic Population ID may have more than one Deme ID, but a Deme ID may ",
      "contain many Synthetic Population IDs." );

  int param_num_demes = 1;
  Params::get_param_from_string( "num_demes", &param_num_demes );
  assert( param_num_demes > 0 );
  Demes.clear();

  std::set< std::string > pop_id_set;

  for ( int d = 0; d < param_num_demes; ++d ) { 
    // allow up to 195 (county) population ids per deme
    // TODO set this limit in param file / Global.h.
    char pop_id_string[ 4096 ];
    std::stringstream ss;
    ss << "synthetic_population_id[" << d << "]";

    if ( Params::does_param_exist( ss.str() ) ) {
      Params::get_indexed_param( "synthetic_population_id", d, pop_id_string );
    }
    else {
      if ( d == 0 ) {
        strcpy( pop_id_string, Global::Synthetic_population_id );
      }
      else {
        Utils::fred_abort( "Help! %d %s %d %s %d %s\n",
            param_num_demes, "demes were requested ( param_num_demes = ",
            param_num_demes, " ), but indexed paramater synthetic_population_id[",
            d,"] was not found!" );
      }
    }
    Demes.push_back( split_by_delim( pop_id_string, delim ) );
    FRED_STATUS( 0, "Split ID string \"%s\" into %d %s using delimiter: \'%c\'\n",
        pop_id_string, int( Demes[ d ].size() ),
        Demes[ d ].size() > 1 ? "tokens" : "token", delim );
    assert( Demes.size() > 0 );
    // only allow up to 255 demes
    assert( Demes.size() <= std::numeric_limits< unsigned char >::max() );
    FRED_STATUS( 0, "Deme %d comprises %d synthetic population id%s:\n",
        d, Demes[ d ].size(), Demes[ d ].size() > 1 ? "s" : "" );
    assert( Demes[ d ].size() > 0 );
    for ( int i = 0; i < Demes[ d ].size(); ++i ) {
      FRED_CONDITIONAL_WARNING(
          pop_id_set.find( Demes[ d ][ i ] ) != pop_id_set.end(),
          "%s %s %s %s\n", "Population ID ", Demes[ d ][ i ],
          "was specified more than once!",
          "Population IDs must be unique across all Demes!" );
      assert( pop_id_set.find( Demes[ d ][ i ] ) == pop_id_set.end() );
      pop_id_set.insert( Demes[ d ][ i ] );
      FRED_STATUS( 0, "--> Deme %d, Synth. Pop. ID %d: %s\n",
          d, i, Demes[ d ][ i ] );
    }
  }
}

void Population::read_all_populations() {
  using namespace std;
  using namespace Utils;
  const char * pop_dir = Global::Synthetic_population_directory;
  assert( Demes.size() > 0 );
  for ( int d = 0; d < Demes.size(); ++d ) { 
    FRED_STATUS( 0, "Loading population for Deme %d:\n", d );
    assert( Demes[ d ].size() > 0 );
    for ( int i = 0; i < Demes[ d ].size(); ++i ) {
      FRED_STATUS( 0 , "Loading population %d for Deme %d: %s\n",
          i, d, Demes[ d ][ i ] );
      // o---------------------------------------- Call read_population to actually
      // |                                         read the population files
      // V
      read_population( pop_dir, Demes[ d ][ i ], "people" );
      if ( Global::Enable_Group_Quarters ) {
        FRED_STATUS( 0, "Loading group quarters population %d for Deme %d: %s\n",
            i, d, Demes[ d ][ i ] );
        // o---------------------------------------- Call read_population to actually
        // |                                         read the population files
        // V
        read_population( pop_dir, Demes[ d ][ i ], "gq_people" );
      }
    }
  }
  // select adult to make health decisions
  Setup_Population_Behavior setup_population_behavior;
  blq.apply( setup_population_behavior );
}

void Population::Setup_Population_Behavior::operator() ( Person & p ) {
  p.setup_behavior();
}

void Population::read_population( const char * pop_dir, const char * pop_id,
    const char * pop_type ) {

  FRED_STATUS(0, "read population entered\n");

  Population::next_id = 0;

  char population_file[1024];
  char line[1024];

  bool is_group_quarters_pop = strcmp( pop_type, "gq_people" ) == 0 ? true : false;

  // try to open compressed population file
  sprintf( population_file, "%s/%s/%s_synth_%s.txt.fsz",
      pop_dir, pop_id, pop_id, pop_type );

  FILE *fp = NULL; 
  fp = Utils::fred_open_file(population_file);
  if (fp != NULL) {
    fclose(fp);
    SnappyFileCompression compressor = SnappyFileCompression( population_file );
    compressor.init_compressed_block_reader();
    // if we have the magic, then it must be fsz block compressed
    if ( compressor.check_magic_bytes() ) {
      // limit to two threads to prevent locking and I/O overhead; also
      // helps to preserve population order in bloque assignment
      #pragma omp parallel
      {
        std::stringstream stream;
        while ( compressor.load_next_block_stream( stream ) ) {
          parse_lines_from_stream( stream, is_group_quarters_pop );
        }
      }
    }
  }
  else {
    // try to find the uncompressed file
    sprintf( population_file, "%s/%s/%s_synth_%s.txt",
        pop_dir, pop_id, pop_id, pop_type );

    fp = Utils::fred_open_file(population_file);
    if (fp == NULL) {
      Utils::fred_abort("population_file %s not found\n", population_file);
    }
    fclose(fp);
    std::ifstream stream ( population_file, ifstream::in );
    parse_lines_from_stream( stream, is_group_quarters_pop );
  }


  FRED_VERBOSE(0, "finished reading population, pop_size = %d\n", pop_size);

}

void Population::update(int day) {

  // clear lists of births and deaths
  if (Global::Enable_Deaths) death_list.clear();
  if (Global::Enable_Births) maternity_list.clear();

  if (Global::Enable_Aging) {

    //Find out if we are currently in a leap year
    int year = Global::Sim_Start_Date->get_year(day);
    int day_of_year = Date::get_day_of_year(year,
        Global::Sim_Start_Date->get_month(day),
        Global::Sim_Start_Date->get_day_of_month(day));

    int bd_count = 0;
    size_t vec_size = 0;

    FRED_VERBOSE( 0, "day_of_year = [%d]\n", day_of_year );

    bool is_leap = Date::is_leap_year(year);

    FRED_VERBOSE( 2, "Day: %d, Year: %d, is_leap: %d\n", day_of_year, year, is_leap ); 

    // All birthdays except Feb. 29 ( unless in leap year ) 
    if ( day_of_year != 60 || is_leap ) {
      for (size_t p = 0; p < birthday_vecs[ day_of_year ].size(); p++) {
        birthday_vecs[ day_of_year ][ p ]->birthday( day );
        bd_count++;
      }
    }

    //If we are NOT in a leap year, then we need to do all of the day 60 (feb 29) birthdays on day 61
    if ( !is_leap && day_of_year == 61 ) {
      for (size_t p = 0; p < birthday_vecs[60].size(); p++) {
        birthday_vecs[ 60 ][ p ]->birthday( day );
        bd_count++;
      }
    }
    FRED_VERBOSE( 0, "birthday count = [%d]\n", bd_count );
  }

  if ( Global::Enable_Births ) {
    // populate the maternity list (Demographics::update_births)
    Update_Population_Births update_population_births( day );
    blq.parallel_masked_apply( fred::Update_Births, update_population_births ); 
    // add the births to the population
    size_t births = maternity_list.size();
    for ( size_t i = 0; i < births; i++ ) {
      Person * mother = maternity_list[ i ];
      Person * baby = mother->give_birth( day );

      if ( Global::Enable_Behaviors ) {
        // turn mother into an adult decision maker, if not already
        if ( mother->is_health_decision_maker() == false ) {
          FRED_VERBOSE( 0, "young mother %d age %d becomes baby's health decision maker on day %d\n",
              mother->get_id(), mother->get_age(), day );
          mother->become_health_decision_maker();
        }
        // let mother decide health behaviors for child
        baby->set_health_decision_maker( mother );
      }

      if ( vacc_manager->do_vaccination() ) {
        FRED_DEBUG( 1, "Adding %d to Vaccine Queue\n",baby->get_id() );
        vacc_manager->add_to_queue( baby );
      }
      int age_lookup = mother->get_age();
      if ( age_lookup > Demographics::MAX_AGE )
        age_lookup = Demographics::MAX_AGE;
      birth_count[ age_lookup ]++;
    }
    FRED_STATUS( 0, "births = %d\n", (int) births );
  }

  if ( Global::Enable_Deaths ) {
    // populate the death list (Demographics::update_deaths)
    Update_Population_Deaths update_population_deaths( day );
    blq.parallel_masked_apply( fred::Update_Deaths, update_population_deaths ); 

    // remove the dead from the population
    size_t deaths = death_list.size();
    for ( size_t i = 0; i < deaths; i++ ) {
      // For reporting
      int age_lookup = death_list[ i ]->get_age();
      if ( age_lookup > Demographics::MAX_AGE )
        age_lookup = Demographics::MAX_AGE;
      if ( death_list[ i ]->get_sex() == 'F' )
        death_count_female[ age_lookup ]++;
      else
        death_count_male[ age_lookup ]++;

      if ( vacc_manager->do_vaccination() ) {
        FRED_DEBUG( 1, "Removing %d from Vaccine Queue\n", death_list[ i ]->get_id() );
        vacc_manager->remove_from_queue( death_list[ i ] );
      }

      // Remove the person from the birthday lists
      if ( Global::Enable_Aging ) {
        map< Person *, int >::iterator itr;
        itr = birthday_map.find( death_list[ i ] );
        if ( itr == birthday_map.end() ) {
          FRED_VERBOSE( 0, "Help! person %d deleted, but not in the birthday_map\n",
              death_list[ i ]->get_id() );
        }
        assert(itr != birthday_map.end());
        int pos = (*itr).second;
        int day_of_year = death_list[ i ]->get_birth_day_of_year();

        // Check to see if the day of the year is after FEB 28
        // and in a leap year, if so, increment position in birthday vector
        if ( day_of_year > 59 && !Date::is_leap_year( death_list[i]->get_birth_year() ) )
          day_of_year++;

        Person * last = birthday_vecs[ day_of_year ].back();
        birthday_map.erase( itr );
        birthday_map[ last ] = pos;

        birthday_vecs[ day_of_year ][ pos ] = birthday_vecs[ day_of_year ].back();
        birthday_vecs[ day_of_year ].pop_back();
      }

      delete_person( death_list[ i ] );
    }
    FRED_STATUS( 0, "deaths = %d\n", (int) deaths );
  }

  // first update everyone's health intervention status
  if ( Global::Enable_Vaccination || Global::Enable_Antivirals ) {
    Update_Health_Interventions update_health_interventions( day );
    blq.apply( update_health_interventions );
  }

  FRED_VERBOSE(1, "population::update health  day = %d\n", day);

  // update everyone's health status
  Update_Population_Health update_population_health( day );
  blq.parallel_masked_apply( fred::Update_Health, update_population_health );
  // Utils::fred_print_wall_time("day %d update_health", day);

  FRED_VERBOSE(1, "population::update household_mobility day = %d\n", day);

  // update household mobility activity on July 1
  if ( Global::Enable_Mobility
      && Date::match_pattern( Global::Sim_Current_Date, "07-01-*" ) ) {
    Update_Population_Household_Mobility update_household_mobility( day );
    blq.apply( update_household_mobility );
  }

  FRED_VERBOSE(1, "population::update prepare activities day = %d\n", day);

  // prepare Activities at start up
  if ( day == 0 ) {
    Prepare_Population_Activities prepare_population_activities( day );
    blq.apply( prepare_population_activities );
    Activities::before_run();
  }

  FRED_VERBOSE(1, "population::update_activity_profile day = %d\n", day);

  // update activity profiles on July 1
  if ( Global::Enable_Aging
      && Date::match_pattern( Global::Sim_Current_Date, "07-01-*" ) ) {
    Update_Population_Activities update_population_activities( day );
    blq.apply( update_population_activities );
  }

  FRED_VERBOSE(1, "population::update_travel day = %d\n", day);

  // update travel decisions
  Travel::update_travel(day);
  // Utils::fred_print_wall_time("day %d update_travel", day);

  FRED_VERBOSE(1, "population::update_behavior day = %d\n", day);

  // update decisions about behaviors
  Update_Population_Behaviors update_population_behaviors( day );
  blq.apply( update_population_behaviors );
  // Utils::fred_print_wall_time("day %d update_behavior", day);

  FRED_VERBOSE(1, "population::update vacc_manager day = %d\n", day);

  // distribute vaccines
  vacc_manager->update(day);
  // Utils::fred_print_wall_time("day %d vacc_manager", day);

  FRED_VERBOSE(1, "population::update av_manager day = %d\n", day);

  // distribute AVs
  av_manager->update(day);
  // Utils::fred_print_wall_time("day %d av_manager", day);

  FRED_STATUS( 1, "population begin_day finished\n");
}

void Population::Update_Population_Births::operator() ( Person & p ) {
  p.update_births( day );
}

void Population::Update_Population_Deaths::operator() ( Person & p ) {
  p.update_deaths( day );
}

void Population::Update_Health_Interventions::operator() ( Person & p ) {
  p.update_health_interventions( day );
}

void Population::Update_Population_Health::operator() ( Person & p ) {
  p.update_health( day );
}

void Population::Update_Population_Household_Mobility::operator() ( Person & p ) {
  p.update_household_mobility();
}

void Population::Prepare_Population_Activities::operator() ( Person & p ) {
  p.prepare_activities();
}

void Population::Update_Population_Activities::operator() ( Person & p ) {
  p.update_activity_profile();
}

void Population::Update_Population_Behaviors::operator() ( Person & p ) {
  p.update_behavior( day );
}

void Population::report(int day) {

  // update infection counters for places
  for (int d = 0; d < Global::Diseases; d++) {
    for (int i = 0; i < pop_size; ++i) {
      if ( !blq.is_valid_index( i ) ) { continue; }
      Person & pop_i = blq.get_item_reference_by_index( i );
      if (pop_i.is_infected(d)) {

	// Update the infection counters for households, if needed for GAIA vis data.
	if (Global::Print_GAIA_Data) {
	  pop_i.update_household_counts(day, d);
	}

	// Update the infection counters for schools
	if (pop_i.get_school() != NULL)
	  pop_i.update_school_counts(day, d);
      }
    }
  }

  // give out anti-virals (after today's infections)
  av_manager->disseminate(day);

  if (Global::Verbose > 0 && Date::match_pattern(Global::Sim_Current_Date, "12-31-*")) {
    // print the statistics on December 31 of each year
    for (int i = 0; i < pop_size; ++i) {
      if ( !blq.is_valid_index( i ) ) { continue; }
      Person & pop_i = blq.get_item_reference_by_index( i );
      int age_lookup = pop_i.get_age();

      if (age_lookup > Demographics::MAX_AGE)
        age_lookup = Demographics::MAX_AGE;
      if (pop_i.get_sex() == 'F')
        age_count_female[age_lookup]++;
      else
        age_count_male[age_lookup]++;
    }
    for (int i = 0; i <= Demographics::MAX_AGE; ++i) {
      int count, num_deaths, num_births;
      double birthrate, deathrate;
      fprintf(Global::Statusfp,
          "DEMOGRAPHICS Year %d TotalPop %d Age %d ", 
          Global::Sim_Current_Date->get_year(), pop_size, i);
      count = age_count_female[i];
      num_births = birth_count[i];
      num_deaths = death_count_female[i];
      birthrate = count>0 ? ((100.0*num_births)/count) : 0.0;
      deathrate = count>0 ? ((100.0*num_deaths)/count) : 0.0;
      fprintf(Global::Statusfp,
          "count_f %d births_f %d birthrate_f %.03f deaths_f %d deathrate_f %.03f ",
          count, num_births, birthrate, num_deaths, deathrate);
      count = age_count_male[i];
      num_deaths = death_count_male[i];
      deathrate = count?((100.0*num_deaths)/count):0.0;
      fprintf(Global::Statusfp,
          "count_m %d deaths_m %d deathrate_m %.03f\n",
          count, num_deaths, deathrate);
      fflush(Global::Statusfp);
    }
    clear_static_arrays();
  }

  for (int d = 0; d < Global::Diseases; d++) {
    disease[d].print_stats(day);
  }

  // Write out the population if the output_population parameter is set.
  // Will write only on the first day of the simulation, on days
  // matching the date pattern in the parameter file, and the on
  // the last day of the simulation
  if(Population::output_population > 0) {
    if((day == 0) || (Date::match_pattern(Global::Sim_Current_Date, Population::output_population_date_match))) {
      this->write_population_output_file(day);
    }
  }
}

void Population::print(int incremental, int day) {
  if (Global::Tracefp == NULL) return;

  if (!incremental){
    if (Global::Trace_Headers) fprintf(Global::Tracefp, "# All agents, by ID\n");
    for (int p = 0; p < pop_size; p++) {
      for (int i=0; i<Global::Diseases; i++) {
        if ( blq.is_valid_index( p ) ) {
          Person & pop_i = blq.get_item_reference_by_index( p );
          pop_i.print(Global::Tracefp, i);
        }
      }
    }

  } else if (1==incremental) {
    ChangeMap::const_iterator iter;

    if (Global::Trace_Headers){
      if (day < Global::Days)
        fprintf(Global::Tracefp, "# Incremental Changes (every %d): Day %3d\n", Global::Incremental_Trace, day);
      else
        fprintf(Global::Tracefp, "# End-of-simulation: Remaining unreported changes\n");

      if (! incremental_changes.size()){
        fprintf(Global::Tracefp, "# <LIST-EMPTY>\n");
        return;
      }
    }

    for (iter = this->incremental_changes.begin();
        iter != this->incremental_changes.end();
        iter++){
      (iter->first)->print(Global::Tracefp, 0); // the map key is a Person*
    }
  } else {
    ChangeMap::const_iterator iter;
    if (Global::Trace_Headers){
      fprintf(Global::Tracefp, "# Agents that never changed\n");
      if (! never_changed.size()){
        fprintf(Global::Tracefp, "# <LIST-EMPTY>\n");
        return;
      }
    }

    for (iter = this->never_changed.begin();
        iter != this->never_changed.end();
        iter++){
      (iter->first)->print(Global::Tracefp, 0); // the map key is a Person*
    }
  }

  // empty out the incremental list of Person's who have changed
  if (-1 < incremental)
    incremental_changes.clear();
}


void Population::end_of_run() {
  // print out agents who have changes yet unreported
  this->print(1, Global::Days);

  // print out all those agents who never changed
  this->print(-1);

  //Write the population to the output file if the parameter is set
  //  * Will write only on the first day of the simulation, days matching the date pattern in the parameter file,
  //    and the last day of the simulation *
  if(Population::output_population > 0) {
    this->write_population_output_file(Global::Days);
  }
}

Disease *Population::get_disease(int disease_id) {
  return &disease[disease_id];
}

void Population::quality_control() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "population quality control check\n");
    fflush(Global::Statusfp);
  }

  // check population
  for (int p = 0; p < pop_size; p++) {
    if ( !blq.is_valid_index( p ) ) { continue; }
    Person & pop_i = blq.get_item_reference_by_index( p );
    if (pop_i.get_household() == NULL) {
      fprintf(Global::Statusfp, "Help! Person %d has no home.\n",p);
      pop_i.print(Global::Statusfp, 0);
    }
  }

  if (Global::Verbose > 0) {
    int n0, n5, n18, n65;
    int count[20];
    int total = 0;
    n0 = n5 = n18 = n65 = 0;
    // age distribution
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < pop_size; p++) {
      if ( !blq.is_valid_index( p ) ) { continue; }
      Person & pop_i = blq.get_item_reference_by_index( p );
      //int a = pop[p]->get_age();
      int a = pop_i.get_age();

      if (a < 5) { n0++; }
      else if (a < 18) { n5++; }
      else if (a < 65) { n18++; }
      else { n65++; }
      int n = a / 5;
      if (n < 20) { count[n]++; }
      else { count[19]++; }
      total++;
    }
    fprintf(Global::Statusfp, "\nAge distribution: %d people\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Global::Statusfp, "age %2d to %d: %6d (%.2f%%)\n",
          5*c, 5*(c+1)-1, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "AGE 0-4: %d %.2f%%\n", n0, (100.0*n0)/total);
    fprintf(Global::Statusfp, "AGE 5-17: %d %.2f%%\n", n5, (100.0*n5)/total);
    fprintf(Global::Statusfp, "AGE 18-64: %d %.2f%%\n", n18, (100.0*n18)/total);
    fprintf(Global::Statusfp, "AGE 65-100: %d %.2f%%\n", n65, (100.0*n65)/total);
    fprintf(Global::Statusfp, "\n");

    // Print out At Risk distribution
    for(int d = 0; d < Global::Diseases; d++){
      if(disease[d].get_at_risk()->get_num_ages() > 0){
        Disease* dis = &disease[d];
        int rcount[20];
        for (int c = 0; c < 20; c++) { rcount[c] = 0; }
        for (int p = 0; p < pop_size; p++) {
          if ( !blq.is_valid_index( p ) ) { continue; }
          Person & pop_i = blq.get_item_reference_by_index( p );
          int a = pop_i.get_age();
          int n = a / 10;
          if(pop_i.get_health()->is_at_risk(dis)==true) {
            if( n < 20 ) { rcount[n]++; }
            else { rcount[19]++; }
          }
        }
        fprintf(Global::Statusfp, "\n Age Distribution of At Risk for Disease %d: %d people\n",d,total);
        for(int c = 0; c < 10; c++ ) {
          fprintf(Global::Statusfp, "age %2d to %2d: %6d (%.2f%%)\n",
              10*c, 10*(c+1)-1, rcount[c], (100.0*rcount[c])/total);
        }
        fprintf(Global::Statusfp, "\n");
      }
    }  
  }
  FRED_STATUS( 0, "population quality control finished\n" );
}

void Population::set_changed(Person *p){
  incremental_changes[p]=true; // note that this element has been changed
  never_changed.erase(p);      // remove it from this list 
  // (might already be gone, but this won't hurt)
}

//Static function to clear arrays
void Population::clear_static_arrays() {
  for (int i = 0; i <= Demographics::MAX_AGE; ++i) {
    age_count_male[i] = 0;
    age_count_female[i] = 0;
    death_count_male[i] = 0;
    death_count_female[i] = 0;
  }
  for (int i = 0; i <= Demographics::MAX_AGE; ++i) {
    birth_count[i] = 0;
  }
}

void Population::assign_classrooms() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign classrooms entered\n");
    fflush(Global::Statusfp);
  }
  for (int p = 0; p < pop_size; p++){
    if ( blq.is_valid_index( p ) ) {
      blq.get_item_reference_by_index( p ).assign_classroom();
    }
  }
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign classrooms finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::assign_offices() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign offices entered\n");
    fflush(Global::Statusfp);
  }
  for (int p = 0; p < pop_size; p++){
    if ( blq.is_valid_index( p ) ) {
      blq.get_item_reference_by_index( p ).assign_office();
    }
  }
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign offices finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::get_network_stats(char *directory) {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "get_network_stats entered\n");
    fflush(Global::Statusfp);
  }
  char filename[FRED_STRING_SIZE];
  sprintf(filename, "%s/degree.csv", directory);
  FILE *fp = fopen(filename, "w");
  fprintf(fp, "id,age,deg,h,n,s,c,w,o\n");
  for (int p = 0; p < pop_size; p++){
    if ( !blq.is_valid_index( p ) ) { continue; }
    Person & pop_i = blq.get_item_reference_by_index( p );
    fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
        pop_i.get_id(),
        pop_i.get_age(),
        pop_i.get_degree(),
        pop_i.get_household_size(),
        pop_i.get_neighborhood_size(),
        pop_i.get_school_size(),
        pop_i.get_classroom_size(),
        pop_i.get_workplace_size(),
        pop_i.get_office_size());
  }
  fclose(fp);
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "get_network_stats finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::report_birth(int day, Person *per) const {
  if (Global::Birthfp == NULL) return;
  fprintf(Global::Birthfp, "day %d mother %d age %d\n",
      day,
      per->get_id(),
      per->get_age());
  fflush(Global::Birthfp);
}

void Population::report_death(int day, Person *per) const {
  if (Global::Deathfp == NULL) return;
  fprintf(Global::Deathfp, "day %d person %d age %d\n",
      day,
      per->get_id(),
      per->get_age());
  fflush(Global::Deathfp);
}

void Population::print_age_distribution(char * dir, char * date_string, int run) {
  FILE *fp;
  int count[21];
  double pct[21];
  char filename[FRED_STRING_SIZE];
  sprintf(filename, "%s/age_dist_%s.%02d", dir, date_string, run);
  printf("print_age_dist entered, filename = %s\n", filename); fflush(stdout);
  for (int i = 0; i < 21; i++) {
    count[i] = 0;
  }
  for (int p = 0; p < pop_size; p++){
    if ( !blq.is_valid_index( p ) ) { continue; }
    Person & pop_i = blq.get_item_reference_by_index( p );
    int age = pop_i.get_age();
    if (0 <= age && age <= Demographics::MAX_AGE) count[age]++;
    if (age > Demographics::MAX_AGE) count[Demographics::MAX_AGE]++;
    assert(age >= 0);
  }
  fp = fopen(filename, "w");
  for (int i = 0; i < 21; i++) {
    pct[i] = 100.0*count[i]/pop_size;
    fprintf(fp, "%d  %d %f\n", i*5, count[i], pct[i]);
  }
  fclose(fp);
}

Person * Population::select_random_person() {
  int i = IRAND(0,pop_size-1);
  while ( !blq.is_valid_index( i ) ) {
    i = IRAND(0,pop_size-1);
  }
  return blq.get_item_pointer_by_index(i);
}

Person * Population::select_random_person_by_age(int min_age, int max_age) {
  int i = IRAND(0,pop_size-1);
  while ( !blq.is_valid_index( i )
      || blq.get_item_reference_by_index( i ).get_age() < min_age
      || blq.get_item_reference_by_index( i ).get_age() > max_age) {
    i = IRAND(0,pop_size-1);
  }
  return blq.get_item_pointer_by_index(i);
}

void Population::write_population_output_file(int day) {

  //Loop over the whole population and write the output of each Person's to_string to the file
  char population_output_file[FRED_STRING_SIZE];
  sprintf(population_output_file, "%s/%s_%s.txt", Global::Output_directory, Population::pop_outfile,
      (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str());
  FILE *fp = fopen(population_output_file, "w");
  if (fp == NULL) {
    Utils::fred_abort("Help! population_output_file %s not found\n", population_output_file);
  }

  //  fprintf(fp, "Population for day %d\n", day);
  //  fprintf(fp, "------------------------------------------------------------------\n");
  for (int p = 0; p < pop_size; ++p) {
    if ( blq.is_valid_index( p ) ) {
      Person & pop_i = blq.get_item_reference_by_index( p );
      fprintf(fp, "%s\n", pop_i.to_string().c_str());
    }
  }
  fflush(fp);
  fclose(fp);
}

void Population::Update_Vaccine_Infection_Counts::operator() (Person & p) {
    Health * h = p.get_health();
    int age = (int) p.get_age();
    if (age > Global::MAX_AGE) {
        age = Global::MAX_AGE;
    }
    if (h->get_symptomatic_date(disease) == day) {
        if (h->is_vaccinated()) {
            sym[age][1]++;
        }
        else {
            sym[age][0]++;
        }
    }
}


void Population::report_vaccine_infection_events(int day) {
    Update_Vaccine_Infection_Counts update_vaccine_infection_counts(day, 0);
    this->blq.masked_apply(fred::Infectious, update_vaccine_infection_counts);
    for (int a=0; a < Global::MAX_AGE; ++a) {
        std::map<int,int> m = update_vaccine_infection_counts.sym[a];
        if (!m.empty()) {
            fprintf(Global::VaccineInfectionTrackerfp,
                "{\"day\": %d, \"age\": %d, \"vaccinated\": %d, \"not_vaccinated\": %d}\n",
                day, a, m[1], m[0]);
            //std::cout << m[0] << "  " << m[1];
        }
    }
}

void Population::Update_Disease_State_By_Block_Counts::operator() (Person & p) {
  Health *h = p.get_health();
  //map_test[1][2]++;
  //Household *house = (Household *) p.get_household();
  //string block = house->get_census_block();
  //Global::Block_Epi_Day_Tracker->increment_index_key_pair("INDEX","N",1);
  //if (p.get_exposure_date(disease) == day) Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"C",1);
  // if (p.is_susceptible(disease)) Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"S",1);
  // if (p.is_infectious(disease)) Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"I",1);
  // if (p.is_infectious(disease) && (h->get_symptomatic_date(disease)>-1))
  //   Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"Is",1);
  // if ((h->get_symptomatic_date(disease) == day)) Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"Cs",1);
  // if (p.is_infected(disease) && !p.is_infectious(disease)) Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"E",1);
  // if (h->get_vaccine_health(0) != NULL){
  // 	if(h->get_vaccine_health(0)->get_vaccination_day()==day){
  // 		Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"V",1); 
  //       }
  // }
  // if (h->get_number_av_taken() > 0){
  //   	if(h->get_av_health(0)->get_av_start_day()==day){
  // 		Global::Block_Epi_Day_Tracker->increment_index_key_pair(block,"Av",1);
  // 	}
  // }
}

void Population::report_disease_state_counts_by_block(int day){
   if(day == 0){
         std::set<string> block_set;
	 for(int p=0; p< this->pop_size;p++){
      
	   if(!this->blq.is_valid_index(p)) {
	     continue;
	   }
	   Person & pop_i = this->blq.get_item_reference_by_index(p);
	   string census_block = ((Household*)pop_i.get_household())->get_census_block();
	   //pop_i.set_household_census_block(*(census_block));
	   block_set.insert(census_block);
	 }
	 
	  for(std::set<string>::iterator census_block_itr = block_set.begin();
	      census_block_itr != block_set.end(); ++census_block_itr) {
	    Global::Block_Epi_Day_Tracker->add_index(*census_block_itr);
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"N",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"E",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"S",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"I",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Is",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"R",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"C",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Cs",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Is",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"V",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Av",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"M",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"N",int(0));
	    Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Day",int(0));
	    
	  }

	  for(int p=0;p<this->pop_size;p++){
	    Person & pop_i = this->blq.get_item_reference_by_index(p);
	    string census_block = ((Household*)pop_i.get_household())->get_census_block();
	    Global::Block_Epi_Day_Tracker->increment_index_key_pair(census_block,"N",int(1));
	    Global::Block_Epi_Day_Tracker->increment_index_key_pair(census_block,"S",int(1));
	  }
   }
	     //printf("!!!!!!!!!Day = 0, initilizing tracker\n");
     // Global::Block_Epi_Day_Tracker->add_index("INDEX);
   //   Global::Block_Epi_Day_Tracker->set_index_key_pair("INDEX","N",int(0));
   //   Global::Block_Epi_Day_Tracker->set_index_key_pair("INDEX","E",int(0));
   //   Global::Block_Epi_Day_Tracker->set_index_key_pair("INDEX","Day",int(0));
   
   //   for (int p=0; p < this->pop_size; p++){
   //     Global::Block_Epi_Day_Tracker->increment_index_key_pair("INDEX","N",1);
   //   }
   // }
    // std::set<string> block_set;
  //   for(int p=0; p< this->pop_size;p++){
      
  //      if(!this->blq.is_valid_index(p)) {
  // 	 continue;
  //      }
  //      Person & pop_i = this->blq.get_item_reference_by_index(p);
  //      string census_block = ((Household*)pop_i.get_household())->get_census_block();
  //      block_set.insert(census_block);
  //   }
    
  //   for(std::set<string>::iterator census_block_itr = block_set.begin();
  // 	census_block_itr != block_set.end(); ++census_block_itr) {
  //     //int census_block_int = atoi((*census_block_itr).c_str()); 
  //     Global::Block_Epi_Day_Tracker->add_index(*census_block_itr);
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"C",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"E",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"I",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"S",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"R",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Cs",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Is",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"V",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Av",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"Day",int(0));
  //     Global::Block_Epi_Day_Tracker->set_index_key_pair(*census_block_itr,"N",int(0));
  //   }
  // }
  // map< int, map<int,int> > map_test;
  // map < int, int> map_inside;
  // map_inside[1] = 0;
  // map_inside[2] = 0;
  // map_test[1] = map_inside;
  //Update_Disease_State_By_Block_Counts update_disease_state_by_block_counts(day,0);
  //this->blq.apply(update_disease_state_by_block_counts);
  //printf("!!!!!!!!!!!!!!!!mapValue = %d\n",map_test[1][2]);
  //Global::Block_Epi_Day_Tracker->copy_for_all_indices("R","N");
  //Global::Block_Epi_Day_Tracker->subtract_for_all_indices("R","S");
  //Global::Block_Epi_Day_Tracker->subtract_for_all_indices("R","E");
  //Global::Block_Epi_Day_Tracker->subtract_for_all_indices("R","I");
}

