//
//
// File: Pop.hpp
//

#ifndef _SYNDEM_POP_H
#define _SYNDEM_POP_H

#include <stack>
#include <set>
#include <new>

#include "Person.hpp"
#include "Random.hpp"
#include "Params.hpp"

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

#endif // _SYNDEM_POP_H
