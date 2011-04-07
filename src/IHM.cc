

#include "IHM.h"
#include "DefaultIHM.h"
#include "FixedIHM.h"
#include "AEB.h"

using namespace std;

void IHM :: setup(Disease *disease)
{
  this->disease = disease;
  max_days = 0;
}

IHM *IHM :: newIHM(int type)
{
  switch(type) {
    case 0: 
      return new DefaultIHM;

    case 1: 
      return new FixedIHM;
    
    case 2: 
      return new AEB;
    
    default:
      printf("Invalid IHM type: %d\n", type);
      abort();
  }
}
