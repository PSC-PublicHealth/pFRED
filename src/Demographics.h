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

class Demographics {
public:
  Demographics(void);
  Demographics(Person* _self, int _age, char _sex, char _occupation,
	       int _marital_status,int _profession);
  ~Demographics(void);
  void reset(void);
  void update(int day);
  int get_age(void)            { return age; }
  int get_age(int day);
  char get_sex(void)           { return sex; }
  char get_occupation(void)    { return occupation; }
  int get_marital_status(void) { return marital_status; }
  int get_profession(void)     { return profession; }
  bool is_pregnant(void)       { return pregnant; }
  void set_occupation(void);
  void print(void);

private:
  Person *self;                // Pointer to the person class belongs
  int init_age;                // Initial age of the agent
  char init_occupation;        // Initial occupation of the agent (depricated)
  int init_marital_status;     // Initial marital status
  int init_profession;         // Initial profession (from census)
  int birthday;                // When the agent was born within the year (not year component)
  int age;                     // Current age of the agent
  char sex;                    // Male or female?
  char occupation;             // Current occupation of the agent (depricated)
  int marital_status;          // Current marital status 
  int profession;              // Current profession (from census)
  bool pregnant;               // Is the agent pregnant
};

#endif // _FRED_DEMOGRAPHICS_H

