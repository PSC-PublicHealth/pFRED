/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Profile.h
//

#ifndef _FRED_PROFILE_H
#define _FRED_PROFILE_H

#define PRESCHOOL_PROFILE 0
#define STUDENT_PROFILE 1
#define TEACHER_PROFILE 2
#define WORKER_PROFILE 3
#define WEEKEND_WORKER_PROFILE 4
#define UNEMPLOYED_PROFILE 5
#define RETIRED_PROFILE 6

class Profile {
  int id;
  double prob_visit[7][20];
  double prob_travel[7];
  
 public:
  Profile() {}
  static void read_profiles(char *filename);
  static bool is_visited(int pos, int prof, int day);
  static bool decide_to_travel(int prof, int day);

 private:
  void set_prob_visit(int day, int place, double p) { prob_visit[day][place] = p; }
  void set_prob_travel(int day, double p) { prob_travel[day] = p; }
  double get_prob_visit(int day, int place) { return prob_visit[day][place]; }
  double get_prob_travel(int day) { return prob_travel[day]; }
};


extern int Profiles;
extern Profile Prof[];

#endif // _FRED_PROFILE_H
