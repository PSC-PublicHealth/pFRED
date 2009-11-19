/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Demographics.hpp
//

#ifndef _FRED_DEMOGRAPHICS_H
#define _FRED_DEMOGRAPHICS_H

class Demographics {
public:
  Demographics(int,char,char,int,int);
  void reset();
  void update(int day);
  int get_age() { return age; }
  int get_age(int day);
  char get_sex() { return sex; }
  char get_occupation() { return occupation; }
  int get_marital_status() { return marital_status; }
  int get_profession() { return profession; }
  void set_occupation();
  void print();

private:
  int init_age;
  char init_occupation;
  int init_marital_status;
  int init_profession;
  int birthday;
  int age;
  char sex;
  char occupation;
  int marital_status;
  int profession;
};

#endif // _FRED_DEMOGRAPHICS_H

