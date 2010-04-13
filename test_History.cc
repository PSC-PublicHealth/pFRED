using namespace std;

#include <iostream>

#include "History.h"

int main (int argc, char** argv){
  History h;
  int strain=16383;

  // size checks, just for fun  
  cout << "History object is " << sizeof(History) << " bytes" << endl;
  cout << "history_event is " << sizeof(struct history_event) << " bytes"
       << endl;

  // pre-fill check
  cout << "Before: " << h.get_num_events() << " events...\n";

  // add some events
  h.add(16382, HE_INFECTED_PLACE, 3, strain);
  h.add(2, HE_INFECTED_BY, 5, strain);
  h.add(3, HE_INFECTION, 'E', strain);
  h.add(4, HE_INFECTION, 'I', strain);
  h.add(5, HE_ANTIVIRAL, 88, strain);
  h.add(5, HE_MUTATION, strain/2, strain);
  h.add(6, HE_INFECTION, 'R', strain);
  h.add(6, HE_IMMUNITY, true, strain);
  h.add(7, HE_SUSCEPTIBILITY, 0.0001, strain); // I'm invincible!

  // check it now...
  cout << "After: " << h.get_num_events() << " events...\n";

  // see what we've got
  cout << h;

  return 0;
}
