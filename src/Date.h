/*
 Copyright 2011 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Date.h
//

#ifndef DATE_H_
#define DATE_H_

#include <string>
#include <vector>

using namespace std;

class Date {

public:

  static const int DEFAULT_MAX_DAYS = 365;

  static const int SUNDAY = 0;
  static const int MONDAY = 1;
  static const int TUESDAY = 2;
  static const int WEDNESDAY = 3;
  static const int THURSDAY = 4;
  static const int FRIDAY = 5;
  static const int SATURDAY = 6;

  static const int JANUARY = 1;
  static const int FEBRUARY = 2;
  static const int MARCH = 3;
  static const int APRIL = 4;
  static const int MAY = 5;
  static const int JUNE = 6;
  static const int JULY = 7;
  static const int AUGUST = 8;
  static const int SEPTEMBER = 9;
  static const int OCTOBER = 10;
  static const int NOVEMBER = 11;
  static const int DECEMBER = 12;

  static const string MMDDYYYY;
  static const string MMDDYY;
  static const string DDMMYYYY;
  static const string DDMMYY;

  Date();
  Date(int max_days);
  Date(int year, int month, int day_of_month);
  Date(int year, int month, int day_of_month, int max_days);

  void set_date(int year, int month, int day_of_month);
  void set_date(int year, int month, int day_of_month, int max_days);
  //void set_date(int year, int month, int day_of_month, int max_days, int day_of_week, int max_days);
  void set_year(int year);
  void set_month(int month);
  void set_day_of_month(int day_of_month);
  void set_day_of_week(int day_of_week);
  int get_year(int t);
  int get_month(int t);
  string get_month_string(int t);
  int get_day_of_month(int t);
  int get_day_of_week(int t);
  string get_day_of_week_string(int t);
  int get_epi_week(int t);
  int get_epi_week_year(int t);

  static int parse_month_from_date_string(string date_string, string format_string);
  static int parse_day_of_month_from_date_string(string date_string, string format_string);
  static int parse_year_from_date_string(string date_string, string format_string);

  virtual ~Date();

private:

  static const int day_table[2][13];
  static const int doomsday_month_val[2][13];

  int year;
  int month;
  int day_of_month;
  int day_of_week;
  vector<int> * day_of_month_vec;
  vector<int> * month_vec;
  vector<int> * year_vec;

  int get_day_of_week();
  void add_to_vectors(int max_days);

  static int get_days_in_month(int month, int year) { return Date::day_table[(Date::is_leap_year(year) ? 1 : 0)][month]; }
  static int get_doomsday_month(int month, int year) { return Date::doomsday_month_val[(Date::is_leap_year(year) ? 1 : 0)][month]; }
  static int get_doomsday_century(int year);
  static bool is_leap_year(int year);
  static int get_day_of_year(int year, int month, int day);
  static int get_day_of_week(int year, int month, int day_of_month);
};

#endif /* DATE_H_ */
