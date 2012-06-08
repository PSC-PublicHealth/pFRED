/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Demographics.h
//

#ifndef _FRED_DEMOGRAPHICS_H
#define _FRED_DEMOGRAPHICS_H

class Person;
class Date;

class Demographics {
public:

  static const int MAX_AGE = 110;
  static const int MAX_PREGNANCY_AGE = 60;
  static const double MEAN_PREG_DAYS = 280.0; //40 weeks
  static const double STDDEV_PREG_DAYS = 7.0; //1 week

  /**
   * Default constructor
   */
  Demographics();

  /**
   * Constructor that sets all of the attributes of a Demographics object
   * @param _self the Person object with which this Demographics object is associated
   * @param _age
   * @param _sex (M or F)
   * @param _marital_status 1 if married, 0 if not
   * @param _profession the code for the Person's occupation
   * @param day the simulation day
   * @param is_newborn needed to know how to set the date of birth
   */
  Demographics(Person* _self, int _age, char _sex, int _marital_status, int rel,
	       int _profession, int day, bool is_newborn = false);

  ~Demographics();

  /**
   * Perform the daily update for this object
   *
   * @param day the simulation day
   */
  void update(int day);

  /**
   * @return the number of days the agent has been alive / 365.0
   */
  double get_real_age();

  /**
   * @return the agent's age
   */
  int get_age()            { return age; }

  /**
   * @return the agent's sex
   */
  char get_sex()           { return sex; }

  /**
   * @return the agent's marital_status
   */
  int get_marital_status() { return marital_status; }

  /**
   * @return the agent's profession
   */
  int get_profession()     { return profession; }

  /**
   * @return <code>true</code> if the agent is pregnant, <code>false</code> otherwise
   */
  bool is_pregnant()       { return pregnant; }

  /**
   * @return <code>true</code> if the agent is deceased, <code>false</code> otherwise
   */
  bool is_deceased()       { return deceased; }

  /**
   * Print out information about this object
   */
  void print();

  /**
   * @return the agent's init_age
   */
  int get_init_age()            { return init_age; }

  /**
   * @return the agent's init_marital_status
   */
  int get_init_marital_status() { return init_marital_status; }

  /**
   * @return the agent's init_profession
   */
  int get_init_profession()     { return init_profession; }
	
  void set_relationship(int rel) { relationship = rel; }
  int get_relationship() { return relationship; }

  /**
   * @return <code>true</code> if the agent is a householder, <code>false</code> otherwise
   */
  bool is_householder() { return relationship == 1; }

  /**
   * Perform the necessary changes to the demographics on an agent's birthday
   */
  void birthday(int day);

  /**
   * @return the agent's birthdate
   */
  Date * get_birthdate() { return this->birthdate; }
  
  void terminate(){}

private:
  Person *self;                // Pointer to the person class belongs
  int init_age;                // Initial age of the agent
  int init_marital_status;     // Initial marital status
  int init_profession;         // Initial profession (from census)
  Date *birthdate;             // When the agent was born
  int deceased_sim_day;        // When the agent (will die) / (died)
  int conception_sim_day;      // When the agent will conceive
  int due_sim_day;             // When the agent will give birth
  int age;                     // Current age of the agent
  char sex;                    // Male or female?
  int marital_status;          // Current marital status 
  int profession;              // Current profession (from census)
  bool pregnant;               // Is the agent pregnant
  bool deceased;               // Is the agent deceased
  int relationship;

  static double age_yearly_mortality_rate_male[MAX_AGE + 1];
  static double age_yearly_mortality_rate_female[MAX_AGE + 1];
  static double age_yearly_birth_rate[MAX_PREGNANCY_AGE + 1];
  static double age_daily_birth_rate[MAX_PREGNANCY_AGE + 1];
  static bool is_initialized;

  /**
   * This method is only used one time during initialization to load the birth rate and mortality rate arrays from files
   */
  void read_init_files();
};

#endif // _FRED_DEMOGRAPHICS_H
