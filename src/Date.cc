/*
 Copyright 2011 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Date.cc
//

#include "Date.h"
#include <time.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <string>

using namespace std;

const int Date::day_table[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

const int Date::doomsday_month_val[2][13] = {
    {0, 31, 28, 7, 4, 9, 6, 11, 8, 5, 10, 7, 12},
    {0, 32, 29, 7, 4, 9, 6, 11, 8, 5, 10, 7, 12}};

const string Date::MMDDYYYY = string("MM/DD/YYYY");
const string Date::MMDDYY = string("MM/DD/YY");
const string Date::DDMMYYYY = string("DD/MM/YYYY");
const string Date::DDMMYY = string("DD/MM/YY");

Date::Date() {

  this->day_of_month_vec = NULL;
  this->month_vec = NULL;
  this->year_vec = NULL;

  //Get the system time
  time_t rawtime;
  time(&rawtime);

  int month, day, year;
  struct tm * curr_time = localtime(&rawtime);

  /*
   * Meaning of values in structure tm:
   * tm_mday day of the month  1-31
   * tm_mon  months since January  0-11
   * tm_year years since 1900
   */
  month = curr_time->tm_mon + 1;
  day = curr_time->tm_mday;
  year = curr_time->tm_year + 1900;

  this->set_date(year, month, day);
}

Date::Date(int max_days) {

  this->day_of_month_vec = NULL;
  this->month_vec = NULL;
  this->year_vec = NULL;

  //Get the system time
  time_t rawtime;
  time(&rawtime);

  int month, day, year;
  struct tm * curr_time = localtime(&rawtime);

  /*
   * Meaning of values in structure tm:
   * tm_mday day of the month  1-31
   * tm_mon  months since January  0-11
   * tm_year years since 1900
   */
  month = curr_time->tm_mon + 1;
  day = curr_time->tm_mday;
  year = curr_time->tm_year + 1900;

  this->set_date(year, month, day, max_days);
}

Date::Date(int year, int month, int day_of_month) {

  this->day_of_month_vec = NULL;
  this->month_vec = NULL;
  this->year_vec = NULL;

  this->set_date(year, month, day_of_month);
}

Date::Date(int year, int month, int day_of_month, int max_days) {

  this->day_of_month_vec = NULL;
  this->month_vec = NULL;
  this->year_vec = NULL;

  this->set_date(year, month, day_of_month, max_days);
}

/**
 * Sets the month, day, and year to the values.  The values will be set
 * in the order Year, Month, then Day.  So, setting Feb, 29, 2009 will
 * result in an error, since the day is outside of the range for that
 * month in that year.
 */
void Date::set_date(int year, int month, int day_of_month) {

  this->set_year(year);
  this->set_month(month);
  this->set_day_of_month(day_of_month);

  this->day_of_month_vec = NULL;
  this->month_vec = NULL;
  this->year_vec = NULL;

  this->add_to_vectors(Date::DEFAULT_MAX_DAYS);

  //Set day_of_week to the actual day of the week that it should be
  this->set_day_of_week(this->get_day_of_week());
}

/**
 * Sets the month, day, and year to the values.  The values will be set
 * in the order Year, Month, then Day.  So, setting Feb, 29, 2009 will
 * result in an error, since the day is outside of the range for that
 * month in that year.
 *
 * By letting the system know how many days it will run, we can build maps
 * of day to year, month and day_of_month
 */
void Date::set_date(int year, int month, int day_of_month, int max_days) {
  this->set_year(year);
  this->set_month(month);
  this->set_day_of_month(day_of_month);

  this->day_of_month_vec = NULL;
  this->month_vec = NULL;
  this->year_vec = NULL;

  this->add_to_vectors(max_days);

  //Set day_of_week to the actual day of the week that it should be
  this->set_day_of_week(this->get_day_of_week());
}

/**
 * Sets the year to the value.  Negative years will just be set to year 0
 */
void Date::set_year(int year) {
  this->year = (year < 0 ? 0 : year);
}

/**
 * Sets the month to the value.  If the month is outside of the legal range (1 [JANUARY] - 12 [DECEMBER]), then
 * abort.
 */
void Date::set_month(int month) {
  if (month >= Date::JANUARY && month <= Date::DECEMBER) {
    this->month = month;
  } else {
    cout << "Help!  Month [" << month << "] is outside of valid range (" << Date::JANUARY <<
        " - " << Date::DECEMBER << ")" << endl;
    abort();
  }
}

/**
 * Sets the day_of_month to the value.  If the day_of_month is outside of the
 * legal range (1-31, 1-30, 1-28(29)), then abort
 */
void Date::set_day_of_month(int day_of_month) {

  if (day_of_month >= 1 && day_of_month <= Date::get_days_in_month(this->month, this->year)) {
    this->day_of_month = day_of_month;
  } else {
    cout << "Help!  Day of month [" << day_of_month << "] is outside of valid range for the month (1 - "
        << this->get_days_in_month(this->month,this->year) << ")" << endl;
    abort();
  }
}

/**
 * Sets the day_of_week to the value.  If the day_of_week is outside of the
 * legal range (0 - 6), then abort
 */
void Date::set_day_of_week(int day_of_week) {

  if (day_of_week >= Date::SUNDAY && day_of_week <= Date::SATURDAY) {
    this->day_of_week = day_of_week;
  } else {
    cout << "Help!  Day of week [" << day_of_week << "] is outside of valid range (" << Date::SUNDAY
        << " - " << Date::SATURDAY << ")" << endl;
    abort();
  }
}

int Date::get_year(int t) {

  if (t > 0) {

    if (this->year_vec  && t < (int)this->year_vec->size()) {
      int * temp = & this->year_vec->at(t);
      return * temp;
    } else {
      this->add_to_vectors(t);

      int * temp = & this->year_vec->at(t);
      return * temp;
    }

  } else {
    return this->year;
  }
}

int Date::get_month(int t) {

  if (t > 0) {

    if (this->month_vec && t < (int)this->month_vec->size()) {
      int * temp = & this->month_vec->at(t);
      return * temp;
    } else {
      this->add_to_vectors(t);

      int * temp = & this->month_vec->at(t);
      return * temp;
    }
  } else {
    return this->month;
  }

  return -1;
}

string Date::get_month_string(int t) {
  int temp_month = this->get_month(t);

  switch(temp_month) {
    case Date::JANUARY:
      return "Jan";
    case Date::FEBRUARY:
      return "Feb";
    case Date::MARCH:
      return "Mar";
    case Date::APRIL:
      return "Apr";
    case Date::MAY:
      return "May";
    case Date::JUNE:
      return "Jun";
    case Date::JULY:
      return "Jul";
    case Date::AUGUST:
      return "Aug";
    case Date::SEPTEMBER:
      return "Sep";
    case Date::OCTOBER:
      return "Oct";
    case Date::NOVEMBER:
      return "Nov";
    case Date::DECEMBER:
      return "Dec";
  }

  return "INVALID MONTH";
}

int Date::get_day_of_month(int t) {

  if(t > 0) {

    if (this->day_of_month_vec
        && t < (int)this->day_of_month_vec->size()) {
      int * temp = & this->day_of_month_vec->at(t);
      return * temp;
    } else {
      this->add_to_vectors(t);

      int * temp = & this->day_of_month_vec->at(t);
      return * temp;
    }

  } else {
    return  this->day_of_month;
  }

}

int Date::get_day_of_week(int t) {
  int ret_value = (this->day_of_week + t) % 7;
  ret_value = (ret_value >= 0 ? ret_value : 7 + ret_value);

  return ret_value;
}

void Date::add_to_vectors(int max_days) {

  size_t vec_size = (size_t) (max_days + 1);

  //If the vectors do not exist, create now
  if (this->year_vec == NULL) {
    this->year_vec = new vector<int>;
    this->year_vec->reserve(vec_size);
  }

  if (this->month_vec == NULL) {
    this->month_vec = new vector<int>;
    this->month_vec->reserve(vec_size);
  }

  if (this->day_of_month_vec == NULL) {
    this->day_of_month_vec = new vector<int>;
    this->day_of_month_vec->reserve(vec_size);
  }

  //Check to see if the vector is empty
  size_t current_vec_size = this->day_of_month_vec->size();
  if (current_vec_size == 0) {
    int current_year = this->year;
    int current_month = this->month;
    int current_day_of_month = this->day_of_month;

    for (int i = 0; i <= max_days; i++) {

      this->year_vec->push_back(current_year);
      this->month_vec->push_back(current_month);
      this->day_of_month_vec->push_back(current_day_of_month);

      if (++current_day_of_month >
        Date::day_table[(Date::is_leap_year(current_year) ? 1 : 0)][current_month]) {
        current_day_of_month = 1;
        if (++current_month > Date::DECEMBER) {
          current_month = Date::JANUARY;
          current_year++;
        }
      }
    }
  } else if (current_vec_size <= (size_t) max_days) {

    int current_year = this->year_vec->back();
    int current_month = this->month_vec->back();
    int current_day_of_month = this->day_of_month_vec->back();//start_day_of_month;

    for (size_t i = 0; i <= ((size_t)max_days - current_vec_size); i++) {

      if (++current_day_of_month >
        Date::day_table[(Date::is_leap_year(current_year) ? 1 : 0)][current_month]) {
        current_day_of_month = 1;
        if (++current_month > Date::DECEMBER) {
          current_month = Date::JANUARY;
          current_year++;
        }
      }

      this->year_vec->push_back(current_year);
      this->month_vec->push_back(current_month);
      this->day_of_month_vec->push_back(current_day_of_month);
    }
  }
}

string Date::get_day_of_week_string(int t) {
  int temp_day_of_week = this->get_day_of_week(t);

  switch(temp_day_of_week) {
    case Date::SUNDAY:
      return "Sun";
    case Date::MONDAY:
      return "Mon";
    case Date::TUESDAY:
      return "Tue";
    case Date::WEDNESDAY:
      return "Wed";
    case Date::THURSDAY:
      return "Thu";
    case Date::FRIDAY:
      return "Fri";
    case Date::SATURDAY:
      return "Sat";
  }

  return "INVALID DAY OF WEEK";
}

int Date::get_day_of_week() {
  return Date::get_day_of_week(this->year, this->month, this->day_of_month);
}

/**
 *  For week and week year, we are using the CDC's definition:
 *
 *  The first day of any MMWR week is Sunday.  MMWR week numbering is
 *  sequential beginning with 1 and incrementing with each week to a
 *  maximum of 52 or 53.  MMWR week #1 of an MMWR year is the first week
 *  of the year that has at least four days in the calendar year.
 *  For example, if January 1 occurs on a Sunday, Monday, Tuesday or
 *  Wednesday, the calendar week that includes January 1 would be MMWR
 *  week #1.  If January 1 occurs on a Thursday, Friday, or Saturday,
 *  the calendar week that includes January 1 would be the last MMWR
 *  week of the previous year (#52 or #53).  Because of this rule,
 *  December 29, 30, and 31 could potentially fall into MMWR week #1
 *  of the following MMWR year.
 */
int Date::get_week(int t) {

  int ret_value = -1;

  //Get the future date info
  int future_year = this->get_year(t);
  int future_month = this->get_month(t);
  int future_day_of_month = this->get_day_of_month(t);
  int future_day_of_year = Date::get_day_of_year(future_year, future_month, future_day_of_month);

  //Figure out on which day of the week Jan 1 occurs for the future date
  int jan_1_day_of_week = this->get_day_of_week((t - (future_day_of_year - 1)));
  int dec_31_day_of_week = this->get_day_of_week(t - (future_day_of_year - 1) + (Date::is_leap_year(future_year) ? 365 : 364));

  if (future_month == Date::DECEMBER && dec_31_day_of_week < Date::WEDNESDAY &&
      future_day_of_month >= (31 - dec_31_day_of_week)) {
    return 1;
  } else {

    int epi_week = (future_day_of_year + jan_1_day_of_week) / 7;
    if (((future_day_of_year + jan_1_day_of_week) % 7) > 0) epi_week++;

    if (jan_1_day_of_week > Date::WEDNESDAY) {
      epi_week--;
      if (epi_week < 1) {
        //Create a new date that represents the last day of the previous year
        Date * tmp_date = new Date(future_year - 1, 12, 31, 1);
        epi_week = tmp_date->get_week(0);
        delete tmp_date;
        return epi_week;
      }
    }

    return epi_week;

  }

  return ret_value;
}

int Date::get_week_year(int t) {

  //Get the future date info
  int future_year = this->get_year(t);
  int future_month = this->get_month(t);
  int future_day_of_month = this->get_day_of_month(t);
  int future_day_of_year = Date::get_day_of_year(future_year, future_month, future_day_of_month);

  //Figure out on which day of the week Jan 1 occurs for the future date
  int jan_1_day_of_week = this->get_day_of_week((t - (future_day_of_year - 1)));

  //Figure out on which day of the week Dec 31 occurs for the future date
  int dec_31_day_of_week = this->get_day_of_week(t - (future_day_of_year - 1) + (Date::is_leap_year(future_year) ? 365 : 364));

  if (future_month == Date::DECEMBER && dec_31_day_of_week < 3 &&
      future_day_of_month >= 31 - dec_31_day_of_week) {
    return future_year + 1;
  } else {

    int epi_week = ceil((future_day_of_year + jan_1_day_of_week) / 7);

    if(jan_1_day_of_week > Date::WEDNESDAY) {
      epi_week--;
      if(epi_week < 1) {
        return future_year - 1;
      }
    }
  }

  return future_year;
}


//Static Methods

bool Date::is_leap_year(int year) {

  if (year%400 == 0)
    return true;
  if (year%100 == 0)
    return false;
  if (year%4 == 0)
    return true;

  return false;
}

int Date::get_doomsday_century(int year) {

  int century = year - (year % 100);
  int r = -1;

  switch (century % 400) {
    case 0:
      r = 2;
      break;
    case 100:
      r = 0;
      break;
    case 200:
      r = 5;
      break;
    case 300:
      r = 3;
      break;
  }

  return r;
}

int Date::get_day_of_year(int year, int month, int day_of_month) {
  int day_of_year = 0;

  for (int i = 1; i < month; i++) {
    day_of_year += Date::day_table[(Date::is_leap_year(year) ? 1 : 0)][i];
  }

  day_of_year += day_of_month;

  return day_of_year;
}

int Date::get_day_of_week(int year, int month, int day_of_month) {
  int x = 0, y = 0;
  int weekday = -1;
  int ddcentury = -1;
  int ddmonth = Date::get_doomsday_month(month, year);
  int century = year - (year % 100);

  ddcentury = Date::get_doomsday_century(year);

  if (ddcentury < 0) return -1;
  if (ddmonth < 0) return -1;
  if (ddmonth > day_of_month) {
    weekday = (7 - ((ddmonth - day_of_month) % 7 ) + ddmonth);
  } else {
    weekday = day_of_month;
  }

  x = (weekday - ddmonth);
  x %= 7;
  y = ddcentury + (year - century) + (floor((year - century)/4));
  y %= 7;
  weekday = (x + y) % 7;

  return weekday;
}

int Date::parse_month_from_date_string(string date_string, string format_string) {

  string temp_str;

  if (format_string.compare(Date::MMDDYYYY) == 0 ||
     format_string.compare(Date::MMDDYY) == 0) {

    size_t pos;
    pos = date_string.find('/');
    if (pos != string::npos) {
      temp_str = date_string.substr(0, pos);
      int i;
      istringstream my_stream(temp_str);

      if(my_stream >> i)
        return i;
    }

  } else if (format_string.compare(Date::DDMMYYYY) == 0 ||
            format_string.compare(Date::DDMMYY) == 0) {

    size_t pos_1, pos_2;
    pos_1 = date_string.find('/');
    if (pos_1 != string::npos) {
      pos_2 = date_string.find('/', pos_1 + 1);
      if (pos_2 != string::npos) {
        temp_str = date_string.substr(pos_1 + 1, pos_2 - pos_1 - 1);
        int i;
        istringstream my_stream(temp_str);

        if(my_stream >> i)
          return i;
      }
    }

  } else {
    cout << "Help!  Unrecognized date format string [" << format_string << "]" << endl;
    abort();
  }

  return -1;
}

int Date::parse_day_of_month_from_date_string(string date_string, string format_string) {

  string temp_str;

  if (format_string.compare(Date::MMDDYYYY) == 0 ||
      format_string.compare(Date::MMDDYY) == 0) {

    size_t pos_1, pos_2;
    pos_1 = date_string.find('/');

    if (pos_1 != string::npos) {
      pos_2 = date_string.find('/', pos_1 + 1);
      if (pos_2 != string::npos) {
        temp_str = date_string.substr(pos_1 + 1, pos_2 - pos_1 - 1);
        int i;
        istringstream my_stream(temp_str);

        if (my_stream >> i)
          return i;
      }
    }

  } else if (format_string.compare(Date::DDMMYYYY) == 0 ||
             format_string.compare(Date::DDMMYY) == 0) {

    size_t pos;
    pos = date_string.find('/');
    if (pos != string::npos) {
      temp_str = date_string.substr(0, pos);
      int i;

      istringstream my_stream(temp_str);

      if (my_stream >> i)
        return i;
    }

  } else {
    cout << "Help!  Unrecognized date format string [" << format_string << "]" << endl;
    abort();
  }

  return -1;

}

int Date::parse_year_from_date_string(string date_string, string format_string) {

  string temp_str;

  if (format_string.compare(Date::MMDDYYYY) == 0 ||
      format_string.compare(Date::DDMMYYYY) == 0 ) {

    size_t pos;
    pos = date_string.find('/', date_string.find('/') + 1);
    if (pos != string::npos) {
      temp_str = date_string.substr(pos + 1);
      int i;

      istringstream my_stream(temp_str);

      if (my_stream >> i)
        return i;

    }
  } else if (format_string.compare(Date::MMDDYY) == 0 ||
             format_string.compare(Date::DDMMYY) == 0) {

    size_t pos;
    pos = date_string.find('/', date_string.find('/') + 1);
    if (pos != string::npos) {
      temp_str = date_string.substr(pos + 1, 2);
      int i;

      istringstream my_stream(temp_str);

      if (my_stream >> i) {
        //70 is the cutoff (i.e any two-digit year greater than or equal to 70 is
        // assumed to be 1900s otherwise, it is 2000s
        i += (i >= 70 ? 1900 : 2000);
        return i;
      }
     }

  } else {
    cout << "Help!  Unrecognized date format string [" << format_string << "]" << endl;
    abort();
  }

  return -1;
}

Date::~Date() {
  if (this->day_of_month_vec) delete day_of_month_vec;
  if (this->month_vec) delete month_vec;
  if (this->year_vec) delete year_vec;
}
