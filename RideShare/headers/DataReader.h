#include <vector>
#include "Ride.h"
#include "Request.h"

#ifndef CSVREADER_H
#define CSVREADER_H

using namespace std;

class DataReader {
  
public:
    vector<Ride> obtainTaxiData();
    vector<Request> obtainHailingData();

private:

};

#endif /* CSVREADER_H */

