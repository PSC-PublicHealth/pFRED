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

  Demographics();
  Demographics(Person* _self, int _age, char _sex,
               int _marital_status,int _profession,
               Date * anchor_date, bool has_random_birthday = true);
  ~Demographics();
  void setup(Date * anchor_date);
  void reset(Date * sim_start_date);
  void update(Date * sim_start_date, int day);
  int get_age()            { return age; }
  double get_real_age(int day);
  char get_sex()           { return sex; }
  int get_marital_status() { return marital_status; }
  int get_profession()     { return profession; }
  bool is_pregnant()       { return pregnant; }
  bool is_deceased()       { return deceased; }
  void set_occupation();
  void print();
	
private:
  Person *self;                // Pointer to the person class belongs
  int init_age;                // Initial age of the agent
  int init_marital_status;     // Initial marital status
  int init_profession;         // Initial profession (from census)
  Date *birthdate;             // When the agent was born
  Date *deceased_date;         // When the agent (will die) / (died)
  Date *conception_date;       // When the agent will conceive
  Date *due_date;              // When the agent will give birth
  int age;                     // Current age of the agent
  char sex;                    // Male or female?
  int marital_status;          // Current marital status 
  int profession;              // Current profession (from census)
  bool pregnant;               // Is the agent pregnant
  bool deceased;               // Is the agent deceased

  static double age_yearly_mortality_rate_male[MAX_AGE + 1];
  static double age_yearly_mortality_rate_female[MAX_AGE + 1];
  static double age_yearly_birth_rate[MAX_PREGNANCY_AGE + 1];
  static double age_daily_birth_rate[MAX_PREGNANCY_AGE + 1];
  static bool is_initialized;

  void read_init_files();
};

#endif // _FRED_DEMOGRAPHICS_H
