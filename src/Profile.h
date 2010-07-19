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

class Profile {
  int id;
  double prob_visit[7][20];
  double prob_travel[7];
  
public:
  Profile() {}
  void set_prob_visit(int day, int place, double p) { prob_visit[day][place] = p; }
  void set_prob_travel(int day, double p) { prob_travel[day] = p; }
  double get_prob_visit(int day, int place) { return prob_visit[day][place]; }
  double get_prob_travel(int day) { return prob_travel[day]; }
};

void read_profiles(char *filename);
int is_visited(int pos, int prof, int day);

extern int Profiles;
extern Profile Prof[];

#endif // _FRED_PROFILE_H
