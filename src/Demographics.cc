/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Demographics.cc
//

#include "Demographics.h"
#include "Population.h"
#include "Age_Map.h"
#include "Person.h"
#include "Random.h"
#include "Global.h"
#include "Date.h"
#include "Utils.h"

class Global;

bool Demographics::is_initialized = false;
double Demographics::age_yearly_mortality_rate_male[Demographics::MAX_AGE + 1];
double Demographics::age_yearly_mortality_rate_female[Demographics::MAX_AGE + 1];
double Demographics::age_yearly_birth_rate[Demographics::MAX_PREGNANCY_AGE + 1];
double Demographics::age_daily_birth_rate[Demographics::MAX_PREGNANCY_AGE + 1];

Demographics::Demographics() {
  //Create the static arrays one time
  if (!Demographics::is_initialized) {
    read_init_files();
    Demographics::is_initialized = true;
  }
  self = NULL;
  init_age = -1;
  age = -1;
  sex = 'n';
  init_marital_status = -1;
  marital_status = -1;
  init_profession = -1;
  profession = -1;
  birthdate = NULL;
  deceased_date = NULL;
  conception_date = NULL;
  due_date = NULL;
  pregnant = false;
  deceased = false;
}

Demographics::Demographics(Person * _self, int _age, char _sex, int _marital_status,
               int _profession, Date * anchor_date, bool has_random_birthday) {

  //Create the static arrays one time
  if (!Demographics::is_initialized) {
    read_init_files();
    Demographics::is_initialized = true;
  }

  self                = _self;
  init_age            = _age;
  age                 = -1;
  sex                 = _sex;
  init_marital_status = _marital_status;
  marital_status      = -1;
  init_profession     = _profession;
  profession          = -1;
  birthdate           = NULL;
  deceased_date       = NULL;
  conception_date     = NULL;
  due_date            = NULL;
  pregnant            = false;
  deceased            = false;

  if (has_random_birthday) {

    double rand_birthday = URAND(0.0, 365.0);
    int anchor_day_of_year = anchor_date->get_day_of_year();
    int birthyear;

    //If the random birthday selected is less than or equal to the system day then the birthyear is
    // found using system_year - init_age, else it is system_year - (init_age + 1)
    if (((int) ceil(rand_birthday)) <= anchor_day_of_year) {
      birthyear = anchor_date->get_year() - init_age;
    } else {
      birthyear = anchor_date->get_year() - (init_age + 1);
    }

    //If the birthyear would have been a leap year, adjust the random number
    // so that it is based on 366 days
    if (Date::is_leap_year(birthyear)) {
      rand_birthday = rand_birthday * 366.0 / 365.0;
    }

    this->birthdate = new Date(birthyear, (int) ceil(rand_birthday));

  } else {

    this->birthdate = new Date(anchor_date->get_year(),
                               anchor_date->get_month(),
                               anchor_date->get_day_of_month());
  }

  setup(anchor_date);
}

Demographics::~Demographics() {
  if (birthdate != NULL) delete birthdate;
  if (deceased_date != NULL) delete deceased_date;
  if (conception_date != NULL) delete conception_date;
  if (due_date != NULL) delete due_date;
}

void Demographics::setup(Date * anchor_date) {

  age                = init_age;
  marital_status     = init_marital_status;
  profession         = init_profession;
  pregnant           = false;
  deceased           = false;

  int age_lookup = (age <= Demographics::MAX_AGE ? age : Demographics::MAX_AGE);

  //Will this person die in the next year?
  if (Global::Enable_Deaths > 0) {
    double pct_chance_to_die = 0.0;

    if (this->sex == 'F')
      pct_chance_to_die = Demographics::age_yearly_mortality_rate_female[age_lookup];
    else
      pct_chance_to_die = Demographics::age_yearly_mortality_rate_male[age_lookup];

    if(URAND(0.0, 1.0) <= pct_chance_to_die) {
      //Yes, so set the death day (in simulation days)
      this->deceased_date = new Date(anchor_date->get_year(),
                                     anchor_date->get_month(),
                                     anchor_date->get_day_of_month());
      this->deceased_date->advance(IRAND(1,364));
    }
  }

  age_lookup = (age <= Demographics::MAX_PREGNANCY_AGE ? age : Demographics::MAX_PREGNANCY_AGE);

  //Is this person pregnant?
  if (Global::Enable_Births > 0) {
    if (this->sex == 'F' && URAND(0.0, 1.0) <= ((0.75) * Demographics::age_yearly_birth_rate[age_lookup])) {

      //Yes, so set the due_date (in simulation days)
      this->due_date = new Date(anchor_date->get_year(),
                                anchor_date->get_month(),
                                anchor_date->get_day_of_month());
      this->due_date->advance(IRAND(1, 280));
      this->pregnant = true;
      // this->self->notify_property_change("pregnant", pregnant);
    }
  }

}

void Demographics::update(Date * sim_start_date, int day) {

  int cur_year = sim_start_date->get_year(day);
  int cur_month = sim_start_date->get_month(day);
  int cur_day_of_month = sim_start_date->get_day_of_month(day);

  //The age to look up should be an integer between 0 and the MAX_AGE
  int age_lookup = (age <= Demographics::MAX_AGE ? age : Demographics::MAX_AGE);

  //Is this day your birthday?
  //Or is your birthday on the leap day and this year is not a leap year?
  if ((this->birthdate->get_month() == cur_month &&
       this->birthdate->get_day_of_month() == cur_day_of_month) ||
      (this->birthdate->get_month() == Date::FEBRUARY &&
       this->birthdate->get_day_of_month() == 29 &&
       cur_month == Date::FEBRUARY &&
       cur_day_of_month == 28 &&
       !Date::is_leap_year(cur_year))) {

    //Notify any classes that have registered as event handlers
    if (Global::Enable_Aging > 0) {
      // int prev_age = age;
      age++;
      // this->self->notify_property_change("age", prev_age, age);
    }

    //Will this person die in the next year?
    if (Global::Enable_Deaths > 0) {
      double pct_chance_to_die = 0.0;

      if (this->sex == 'F')
        pct_chance_to_die = Demographics::age_yearly_mortality_rate_female[age_lookup];
      else
        pct_chance_to_die = Demographics::age_yearly_mortality_rate_male[age_lookup];

      if(!this->deceased &&
         this->deceased_date == NULL &&
         URAND(0.0, 1.0) <= pct_chance_to_die) {

        //Yes, so set the death day (in simulation days)
        this->deceased_date = new Date(cur_year, cur_month, cur_day_of_month);
        this->deceased_date->advance(IRAND(0,364));
      }
    }
  }

  //The age to look up should be an integer between 0 and the MAX_PREGNANCY_AGE
  age_lookup = (age <= Demographics::MAX_PREGNANCY_AGE ? age : Demographics::MAX_PREGNANCY_AGE);

  //Is this your day to conceive?
  if (this->sex == 'F' && age <= MAX_PREGNANCY_AGE &&
      !this->pregnant &&
      this->conception_date == NULL &&
      this->due_date == NULL &&
      URAND(0.0, 1.0) <= Demographics::age_daily_birth_rate[age]) {
    this->conception_date = new Date(cur_year, cur_month, cur_day_of_month);
    this->due_date = new Date(cur_year, cur_month, cur_day_of_month);
    double random_due_date = draw_normal(Demographics::MEAN_PREG_DAYS, Demographics::STDDEV_PREG_DAYS);
    this->due_date->advance((int)(random_due_date + 0.5)); //round the random_due_date
    this->pregnant = true;
    //Notify any classes that have registered as event handlers
    // this->self->notify_property_change("pregnant", true);
  }

  //Is this your day to give birth
  if(Global::Enable_Births &&
     this->sex == 'F' &&
     this->pregnant &&
     this->due_date != NULL &&
     this->due_date->get_year() == cur_year &&
     this->due_date->get_month() == cur_month &&
     this->due_date->get_day_of_month() == cur_day_of_month) {
    if (this->conception_date != NULL) {
      delete this->conception_date;
      this->conception_date = NULL;
    }
    delete this->due_date;
    this->due_date = NULL;
    this->pregnant = false;
    //Notify any classes that have registered as event handlers
    // this->self->notify_property_change("pregnant", false);
    // this->self->notify_property_change("deliver", true);
    self->get_population()->prepare_to_give_birth(day, self);
  }

  //Is this your day to die?
  if(Global::Enable_Deaths > 0 &&
     this->deceased_date != NULL &&
     this->deceased_date->get_year() == cur_year &&
     this->deceased_date->get_month() == cur_month &&
     this->deceased_date->get_day_of_month() == cur_day_of_month) {
    this->deceased = true;
    //Notify any classes that have registered as event handlers
    // this->self->notify_property_change("deceased", true);
    self->get_population()->prepare_to_die(day, self);
  }

}

void Demographics::read_init_files() {

  char yearly_mortality_rate_file[256];
  char yearly_birth_rate_file[256];

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "read demographic init files entered\n"); fflush(Global::Statusfp);
  }

  get_param((char *) "yearly_mortality_rate_file", yearly_mortality_rate_file);
  get_param((char *) "yearly_birth_rate_file", yearly_birth_rate_file);

  // read death rate file and load the values unt the death_rate_array
  FILE *fp = fopen(yearly_mortality_rate_file, "r");
  if (fp == NULL) {
    fprintf(Global::Statusfp, "Demographic init_file %s not found\n", yearly_mortality_rate_file);
    exit(1);
  }

  for (int i = 0; i <= Demographics::MAX_AGE; i++) {
    int age;
    float female_rate;
    float male_rate;

    if (fscanf(fp, "%d %f %f",
               &age, &female_rate, &male_rate) != 3) {
      Utils::fred_abort("Help! Read failure for age %d\n", i); 
    }

    Demographics::age_yearly_mortality_rate_female[i] = (double)female_rate;
    Demographics::age_yearly_mortality_rate_male[i] = (double)male_rate;
  }

  fclose(fp);

  //Now read and load the birth rates
  fp = fopen(yearly_birth_rate_file, "r");

  if (fp == NULL) {
    fprintf(Global::Statusfp, "Demographic init_file %s not found\n", yearly_birth_rate_file);
    exit(1);
  }
  for (int i = 0; i <= Demographics::MAX_PREGNANCY_AGE; i++) {
    int age;
    float rate;

    if (fscanf(fp, "%d %f",
               &age, &rate) != 2) {
      Utils::fred_abort("Help! Read failure for age %d\n", i); 
    }

    Demographics::age_yearly_birth_rate[i] = (double)rate;
  }

  for (int i = 0; i <= Demographics::MAX_PREGNANCY_AGE; i++) {
    Demographics::age_daily_birth_rate[i] = Demographics::age_yearly_birth_rate[i] / 365.25;
  }

  fclose(fp);
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "finished reading Demographic init_file = %s\n", yearly_mortality_rate_file);
    fflush(Global::Statusfp);
  }
}


void Demographics::reset(Date * sim_start_date) {

  //Capture the values prior to reseting
  int prev_age       = age;

  age                = init_age;
  marital_status     = init_marital_status;
  profession         = init_profession;
  pregnant           = false;
  deceased           = false;

  if (deceased_date != NULL) {
    delete deceased_date;
    deceased_date = NULL;
  }

  if (due_date != NULL) {
    delete due_date;
    due_date = NULL;
  }

  if (conception_date != NULL) {
    delete conception_date;
    conception_date = NULL;
  }

  //If we are going back to a younger age, notify any classes that have registered as event handlers
  if (Global::Enable_Aging > 0 && prev_age > age) {
    // this->self->notify_property_change("age", prev_age, age);
  }

  int age_lookup = (age <= Demographics::MAX_AGE ? age : Demographics::MAX_AGE);

  //Will this person die in the next year?
  if (Global::Enable_Deaths > 0) {
    double pct_chance_to_die = 0.0;

    if (this->sex == 'F')
      pct_chance_to_die = Demographics::age_yearly_mortality_rate_female[age_lookup];
    else
      pct_chance_to_die = Demographics::age_yearly_mortality_rate_male[age_lookup];

    if(URAND(0.0, 1.0) <= pct_chance_to_die) {
      //Yes, so set the death day (in simulation days)
      this->deceased_date = new Date(sim_start_date->get_year(),
                                   sim_start_date->get_month(),
                                   sim_start_date->get_day_of_month());
      this->deceased_date->advance(IRAND(1,364));
    }
  }

  age_lookup = (age <= Demographics::MAX_PREGNANCY_AGE ? age : Demographics::MAX_PREGNANCY_AGE);
  //Is this person pregnant?
  if (Global::Enable_Births > 0) {
    if (this->sex == 'F' && URAND(0.0, 1.0) <= ((0.75) * Demographics::age_yearly_birth_rate[age_lookup])) {

      //Yes, so set the due_date (in simulation days)
      this->due_date = new Date(sim_start_date->get_year(),
                              sim_start_date->get_month(),
                              sim_start_date->get_day_of_month());
      this->due_date->advance(IRAND(1, 280));
      this->pregnant = true;
      // this->self->notify_property_change("pregnant", pregnant);
    }
  }
}

void Demographics::print() {
}

double Demographics::get_real_age(int day) {
  Date * today = new Date(Global::Sim_Date->get_year(day),
                          Global::Sim_Date->get_month(day),
                          Global::Sim_Date->get_day_of_month(day));
  int days_of_life = Date::days_between(today, birthdate);
  delete today;
  return ((double) days_of_life / 365.0);
}

