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
  static const int DEFAULT_MAX_DAYS = 200000;
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
  static const string YYYYMMDD;
  static const string MMDDYYYY;
  static const string MMDDYY;
  static const string DDMMYYYY;
  static const string DDMMYY;
  static const int INVALID = -1;

  /**
   * Default constructor
   */
  Date();

  /**
   *
   */
  Date(string date_string, string format_string = Date::YYYYMMDD);
  Date(int year, int day_of_year);
  Date(int year, int month, int day_of_month);
  void set_date(int year, int month, int day_of_month);
  int get_year() { return this->get_year(0); }
  int get_year(int t);
  int get_month() { return this->get_month(0); }
  int get_month(int t);
  string get_month_string() { return this->get_month_string(0); }
  string get_month_string(int t);
  int get_day_of_month() { return this->get_day_of_month(0); }
  int get_day_of_month(int t);
  int get_day_of_year() { return this->get_day_of_year(0); }
  int get_day_of_year(int t);
  int get_day_of_week();
  int get_day_of_week(int t);
  string get_day_of_week_string() { return this->get_day_of_week_string(0); }
  string get_day_of_week_string(int t);
  int get_epi_week() { return this->get_epi_week(0); }
  int get_epi_week(int t);
  int get_epi_week_year()  { return this->get_epi_week_year(0); }
  int get_epi_week_year(int t);
  int get_days_since_jan_1_1600() { return this->days_since_jan_1_1600; }
  void advance() { this->days_since_jan_1_1600++; }
  void advance(int days) { this->days_since_jan_1_1600+=days; }
  Date * clone();
  bool equals(Date * check_date);
  int compare_to(Date * check_date);
  string to_string();
  virtual ~Date();
  void setup(char * output_directory, int days);
  string get_YYYYMMDD(int day);
  string get_YYYYMM(int day);
  string get_MMDD(int day);

  static int days_between(Date * date_1, Date * date_2);
  static int days_between(int sim_day, Date * date_2);
  static bool is_leap_year(int year);
  static int get_day_of_year(int year, int month, int day);
  static int parse_month_from_date_string(string date_string, string format_string);
  static int parse_day_of_month_from_date_string(string date_string, string format_string);
  static int parse_year_from_date_string(string date_string, string format_string);
  static bool day_is_between_MMDD(char * current, char * start, char * end);
  static bool match_pattern(int simulation_day, string pattern);
  static bool day_in_range_MMDD(int day, char * start_day, char * end_day);
  static int get_current_year(int day);
  static int get_current_month(int day);
  static int get_current_day_of_month(int day);
  static int get_current_day_of_week(int day);
  static Date * new_date(int day);
private:
  static const int day_table[2][13];
  static const int doomsday_month_val[2][13];
  static vector<int> *day_of_month_vec;
  static vector<int> *month_vec;
  static vector<int> *year_vec;
  static bool is_initialized;
  int days_since_jan_1_1600;
  static void initialize_vectors();
  static void add_to_vectors(int days_since_jan_1_1600);
  static int get_doomsday_month(int month, int year)
  { return Date::doomsday_month_val[(Date::is_leap_year(year) ? 1 : 0)][month]; }
  static int get_doomsday_century(int year);
  static int get_day_of_week(int year, int month, int day_of_month);
  //char date_string[32];
};

#endif /* DATE_H_ */
