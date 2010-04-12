using namespace std;

#include <iostream>

#include "History.h"

int main (int argc, char** argv){
  History h;
  int strain=3;

  // must set a parser at some point before dumping anything
  h.set_parser(history_parser_health);

  // pre-fill check
  cout << "Before: " << h.get_num_events() << " events...\n";

  // add some events
  h.add(2, HTH_INFECTED_PLACE, strain, 3);
  h.add(2, HTH_INFECTED_BY, strain, 5);
  h.add(3, HTH_INFECTION, strain, 'E');
  h.add(4, HTH_INFECTION, strain, 'I');
  h.add(5, HTH_ANTIVIRAL, strain, 88);
  h.add(6, HTH_INFECTION, strain, 'R');
  h.add(6, HTH_IMMUNITY, strain, true);

  // check it now...
  cout << "After: " << h.get_num_events() << " events...\n";

  // see what we've got
  h.dump();

  return 0;
}
