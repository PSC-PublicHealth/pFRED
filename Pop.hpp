/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Pop.hpp
//

#ifndef _SYNDEM_POP_H
#define _SYNDEM_POP_H

#include <stack>
#include <set>
#include <new>

#include "Random.hpp"
#include "Params.hpp"
#include "Person.hpp"
#include "Profile.hpp"

// From Disease.hpp
int get_diseases();

void get_population_parameters();
void setup_population();
void read_population();
void read_schedules();
void set_occupations();
void reset_population(int run);
void population_quality_control();
void update_exposed_population(int day);
void update_infectious_population(int day);
void update_population_stats(int day);
void print_population_stats(int day);
void print_population();
void insert_into_exposed_list(int d, int p);
void insert_into_infectious_list(int d, int p);
void remove_from_exposed_list(int d, int p);
void remove_from_infectious_list(int d, int p);
void start_outbreak();
int get_age(int per);
int get_role(int per, int dis);
char get_disease_status(int per, int dis);
int is_place_on_schedule_for_person(int per, int day, int loc);
double get_infectivity(int per, int dis);
double get_susceptibility(int per, int dis);
char get_disease_status(int per, int dis);
void make_exposed(int per, int dis, int infector, int loc, char type, int day);
void add_infectee(int per, int dis);
void update_schedule(int per, int day);
void get_schedule(int per, int *n, int *schedule);
double get_attack_rate(int dis);

#endif // _SYNDEM_POP_H
