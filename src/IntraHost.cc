

#include "IntraHost.h"
#include "DefaultIntraHost.h"
#include "FixedIntraHost.h"
//#include "ODEIntraHost.h"

using namespace std;

void IntraHost :: setup(Disease *disease) {
  this->disease = disease;
  max_days = 0;
  }

IntraHost *IntraHost :: newIntraHost(int type) {
  switch(type) {
    case 0:
      return new DefaultIntraHost;

    case 1:
      return new FixedIntraHost;

      //    case 2:
      //      return new ODEIntraHost;

    default:
      printf("Invalid IntraHost type: %d\n", type);
      abort();
    }
  }

int IntraHost :: get_days_symp() {
  return 0;
  }
