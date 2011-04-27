/*
 * Person_Event_Interface.h
 *
 *  Created on: Feb 18, 2011
 *      Author: ddg5
 */

#ifndef FRED_PERSON_EVENT_INTERFACE_H_
#define FRED_PERSON_EVENT_INTERFACE_H_

#include <string>

using namespace std;

class Person;

class Person_Event_Interface {
  public:

    Person_Event_Interface();
    virtual ~Person_Event_Interface();

    virtual void handle_property_change_event(Person *source, string property_name, int prev_val, int new_val) = 0;
    virtual void handle_property_change_event(Person *source, string property_name, bool new_val) = 0;
  };

#endif /* FRED_PERSON_EVENT_INTERFACE_H_ */
