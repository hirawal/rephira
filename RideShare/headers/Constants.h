#include <vector>
#include "Ride.h"
#include "Request.h"

#ifndef CONSTANTS_H
#define CONSTANTS_H

using namespace std;

class Constants{

    float waitingTime = 300.0; // 5 mins
    float delay = 600.00; //10 mins
  
public:
  float getMaxDelay();
  float getMaxWaitingTime();

private:
  
};

#endif /* CONSTANTS_H*/

