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
// File: Activities.h
//

#ifndef _FRED_ACTIVITIES_H
#define _FRED_ACTIVITIES_H

#include <vector>
#include <bitset>

#include "Global.h"
#include "Random.h"
#include "Epidemic.h"

class Person;
class Place;

// the following enum defines symbolic names for
// activities.  The last element should always be
// FAVORITE_PLACES

enum Activity_index {
  HOUSEHOLD_ACTIVITY,
  NEIGHBORHOOD_ACTIVITY,
  SCHOOL_ACTIVITY,
  CLASSROOM_ACTIVITY,
  WORKPLACE_ACTIVITY,
  OFFICE_ACTIVITY,
  FAVORITE_PLACES,
};

#define MAX_MOBILITY_AGE 100

// Activity Profiles
#define INFANT_PROFILE 'I'
#define PRESCHOOL_PROFILE 'P'
#define STUDENT_PROFILE 'S'
#define COLLEGE_STUDENT_PROFILE 'C'
#define TEACHER_PROFILE 'T'
#define WORKER_PROFILE 'W'
#define WEEKEND_WORKER_PROFILE 'N'
#define UNEMPLOYED_PROFILE 'U'
#define RETIRED_PROFILE 'R'
#define PRISON_PROFILE 'J'
#define MILITARY_PROFILE 'M'
#define UNDEFINED_PROFILE 'X'

class Activities {
public:
  Activities (Person * self, Place *house, Place *school, Place *work);

  /**
   * Setup activities at start of run
   */
  void prepare();

  /**
   * Setup sick leave depending on size of workplace
   */
  void initialize_sick_leave();

  /**
   * Assigns an activity profile to the agent
   */
  void assign_profile( Person * self );

  /**
   * Perform the daily update for an infectious agent
   *
   * @param day the simulation day
   */
  void update_infectious_activities( Person * self, int day, int dis );

  /**
   * Perform the daily update for a susceptible agent
   *
   * @param day the simulation day
   */
  void update_susceptible_activities( Person * self, int day, int dis );

  /**
   * Perform the daily update to the schedule
   *
   * @param day the simulation day
   */
  void update_schedule( Person * self, int day );

  /**
   * Decide whether to stay home if symptomatic.
   * May depend on availability of sick leave at work.
   *
   * @param day the simulation day
   */
  void decide_whether_to_stay_home( Person * self, int day );

  /**
   * Decide whether to stay home if symptomatic.
   * If Enable_default_sick_leave_behavior is set, the decision is made only once,
   * and the agent stays home for the entire symptomatic period, or never stays home.
   */
  bool default_sick_leave_behavior();

  /// returns string containing Activities schedule; does
  /// not include trailing newline
  std::string schedule_to_string( Person * self, int day );

  /**
   * Print the Activity schedule
   */
  void print_schedule( Person * self, int day );

  /// returns string containing Activities status; does
  /// not include trailing newline
  std::string to_string( Person * self );
  std::string to_string();

  /**
   * Print out information about this object
   */
  void print( Person * self );

  unsigned char get_deme_id(); 

  Place * get_favorite_place(int i) {
    return favorite_place[place_map[i]]; 
  }

  void set_favorite_place(int i, Place * p) {
    if (p == NULL) {
      if (place_map[i] != 0) {
        // printf("Before %s\n", to_string().c_str());fflush(stdout);
        // release pointer to favorite place, to avoid memory leak
        int last = favorite_place.size() - 1;
        Place * tmp = favorite_place[last];
        favorite_place[place_map[i]] = tmp;
        for (int j = 0; j < FAVORITE_PLACES; j++) {
          if (place_map[j] == last) {
            place_map[j] = place_map[i];
            break;
          }
        }
        favorite_place.pop_back();
      }
      place_map[i] = 0;
      // printf("After %s\n\n", to_string().c_str());fflush(stdout);
    }
    else {
      if (place_map[i] == 0) {
        favorite_place.push_back(NULL);
        place_map[i] = favorite_place.size()-1;
      }
      favorite_place[place_map[i]] = p;
    }
  }

  void set_household(Place * p) {
    set_favorite_place(HOUSEHOLD_ACTIVITY, p);
  }

  void set_neighborhood(Place * p) {
    set_favorite_place(NEIGHBORHOOD_ACTIVITY, p);
  }

  void set_school(Place * p) {
    set_favorite_place(SCHOOL_ACTIVITY, p);
  }

  void set_classroom(Place * p) {
    set_favorite_place(CLASSROOM_ACTIVITY, p);
  }

  void set_workplace(Place * p) {
    set_favorite_place(WORKPLACE_ACTIVITY, p);
  }

  void set_office(Place * p) {
    set_favorite_place(OFFICE_ACTIVITY, p);
  }


  Place * get_temporary_household() {
    return tmp_favorite_place[place_map[HOUSEHOLD_ACTIVITY]];
  }


  /**
   * @return a pointer to this agent's Household
   *
   * If traveling, this is the household that is currently
   * being visited
   */
  Place * get_household() {
    return get_favorite_place(HOUSEHOLD_ACTIVITY);
  }

  /**
   * @return a pointer to this agent's permanent Household
   *
   * If traveling, this is the Person's permanent residence,
   * NOT the household being visited
   */
  Place * get_permanent_household() {
    if ( travel_status == true && traveling_outside == true ) {
      return get_temporary_household();
    }
    else {
      return get_household();
    }
  }

  /**
   * @return a pointer to this agent's Neighborhood
   */
  Place * get_neighborhood() {
    return get_favorite_place(NEIGHBORHOOD_ACTIVITY);
  }


  /**
   * @return a pointer to this agent's School
   */
  Place * get_school() {
    return get_favorite_place(SCHOOL_ACTIVITY);
  }


  /**
   * @return a pointer to this agent's Classroom
   */
  Place * get_classroom() {
    return get_favorite_place(CLASSROOM_ACTIVITY);
  }


  /**
   * @return a pointer to this agent's Workplace
   */
  Place * get_workplace() {
    return get_favorite_place(WORKPLACE_ACTIVITY);
  }

  /**
   * @return a pointer to this agent's Office
   */
  Place * get_office() {
    return get_favorite_place(OFFICE_ACTIVITY);
  }

  /**
   * Assign the agent to a School
   */
  void assign_school( Person * self );

  /**
   * Assign the agent to a Classroom
   */
  void assign_classroom( Person * self );

  /**
   * Assign the agent to a Workplace
   */
  void assign_workplace( Person * self );

  /**
   * Assign the agent to an Office
   */
  void assign_office( Person * self );

  /**
   * Update the agent's profile
   */
  void update_profile( Person * self );

  /**
   * Update the household mobility of the agent</br>
   * Note: this is not used unless <code>Global::Enable_Mobility</code> is set to <code>true</code>
   */
  void update_household_mobility( Person * self );

  /**
   * Unenroll from all the favorite places
   */
  void terminate( Person * self );

  /**
   * Increments the incidence count for a given disease and its strains for all of the agent's favorite places
   */
  void addIncidence(int disease, std::vector<int> strains);

  /**
   * Increments the prevalence count for a given disease and its strains for all of the agent's favorite places
   */
  void addPrevalence(int disease, std::vector<int> strains);

  /**
   * The agent begins traveling.  The favorite places for this agent are stored, and it gets a new schedule
   * based on the agent it is visiting.
   *
   * @param visited a pointer to the Person object being visited
   * @see Activities::store_favorite_places()
   */
  void start_traveling( Person * self, Person * visited );

  /**
   * The agent stops traveling and returns to its original favorite places
   * @see Activities::restore_favorite_places()
   */
  void stop_traveling( Person * self );

  /**
   * @return <code>true</code> if the agent is traveling, <code>false</code> otherwise
   */
  bool get_travel_status() { return travel_status; }
  
  bool become_a_teacher(Person *self, Place *school);

  /**
   * Return the number of other agents in an agent's neighborhood, school,
   * and workplace.
   */
  int get_degree();

  int get_household_size();
  int get_group_size(int index) {
    int size = 0;
    if (get_favorite_place(index) != NULL)
      size = get_favorite_place(index)->get_size();
    return size;
  }

  bool is_sick_leave_available() { return sick_leave_available; }
  int get_sick_days_absent() { return my_sick_days_absent; }
  int get_sick_days_present() { return my_sick_days_present; }
  static void update(int day);
  static void end_of_run();
  static void before_run();
  bool is_teacher() { return profile == TEACHER_PROFILE; }
  bool is_student() { return profile == STUDENT_PROFILE; }

private:

  unsigned char place_map[FAVORITE_PLACES];
  vector <Place *> favorite_place;
  Place ** tmp_favorite_place; // list of favorite places, stored while traveling
  Place * home_neighborhood;
  std::bitset< FAVORITE_PLACES > on_schedule; // true iff favorite place is on schedule
  int schedule_updated;                         // date of last schedule update
  bool travel_status;                                // true if traveling
  bool traveling_outside;      // true if traveling outside modeled area
  char profile;                                      // activities profile type

  // individual sick day variables
  short int my_sick_days_absent;
  short int my_sick_days_present;
  float sick_days_remaining;
  bool sick_leave_available;
  bool my_sick_leave_decision_has_been_made;
  bool my_sick_leave_decision;

  // static variables
  static bool is_initialized; // true if static arrays have been initialized
  static bool is_weekday;     // true if current day is Monday .. Friday
  static int day_of_week;     // day of week index, where Sun = 0, ... Sat = 6
  static double age_yearly_mobility_rate[MAX_MOBILITY_AGE + 1];

  // run-time parameters
  static double Community_distance;     // size of community (in km)
  static double Community_prob;         // prob of visiting Community per day
  static double Home_neighborhood_prob; // prob of visiting home neighborhood per day

  static int Enable_default_sick_behavior;
  static double Default_sick_day_prob;
  // mean number of sick days taken if sick leave is available
  static double SLA_mean_sick_days_absent;
  // mean number of sick days taken if sick leave is unavailable
  static double SLU_mean_sick_days_absent;
  // prob of taking sick days if sick leave is available
  static double SLA_absent_prob;
  // prob of taking sick days if sick leave is unavailable
  static double SLU_absent_prob;
  // extra sick days for fle
  static double Flu_days;

  // sick days statistics
  static int Sick_days_present;
  static int Sick_days_absent;
  static int School_sick_days_present;
  static int School_sick_days_absent;

  /**
   * Reads the yearly_mobility_rate_file set in params.def
   * Note: this is not used unless <code>Global::Enable_Mobility</code> is set to <code>true</code>
   */
  void read_init_files();

  void clear_favorite_places() {
    for (int i = 0; i < FAVORITE_PLACES; i++) {
      place_map[i] = 0;
    }
    favorite_place.clear();
    favorite_place.push_back(NULL);
  }

  void enroll_in_favorite_places(Person *self) {
    for (int i = 0; i < FAVORITE_PLACES; i++) {
      if (get_favorite_place(i) != NULL) {
        get_favorite_place(i)->enroll(self);
      }
    }
  }

  void unenroll_from_favorite_places(Person * self) {
    for (int i = 0; i < FAVORITE_PLACES; i++) {
      if (get_favorite_place(i) != NULL) {
        get_favorite_place(i)->unenroll(self);
      }
    }
  }

  void make_favorite_places_infectious(Person *self, int dis) {
    for (int i = 0; i < FAVORITE_PLACES; i++) {
      if (on_schedule[i]) {
        assert(get_favorite_place(i) != NULL);
        get_favorite_place(i)->add_infectious(dis, self);
      }
    }
  }

  void join_susceptible_lists_at_favorite_places(Person * self, int dis) {
    for (int i = 0; i < FAVORITE_PLACES; i++) {
      if (on_schedule[i] && i != HOUSEHOLD_ACTIVITY) {
        assert(get_favorite_place(i) != NULL);
        if (get_favorite_place(i)->is_infectious(dis)) {
          get_favorite_place(i)->add_susceptible(dis, self);
        }
      }
    }
  }

  /**
   * Place this agent's favorite places into a temporary location
   */

  void store_favorite_places() {
    tmp_favorite_place = new Place * [favorite_place.size()]; 
    for (int i = 0; i < favorite_place.size(); i++) {
      tmp_favorite_place[i] = favorite_place[i];
    }
  }

  /**
   * Copy the favorite places from the temporary location, then reclaim the allocated memory of the temporary storage
   */
  void restore_favorite_places() {
    for (int i = 0; i < favorite_place.size(); i++) {
      favorite_place[i] = tmp_favorite_place[i];
    }
    delete [] tmp_favorite_place;
  }

  int get_place_id(int p) {
    return get_favorite_place(p)==NULL? -1 : get_favorite_place(p)->get_id();
  }

  char * get_place_label(int p) {
    return get_favorite_place(p)==NULL? (char *) "NULL" : get_favorite_place(p)->get_label();
  }


protected:

  /**
   * Default constructor
   */
  friend class Person;
  Activities() { }

};


#endif // _FRED_ACTIVITIES_H
