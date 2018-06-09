#include <vector>
#include "Request.h"
#include <string>

#ifndef TRIP_H
#define TRIP_H

using namespace std;

class Trip{

   vector<Request> tripPassengers; 
   float tripCost;

public:
   
   void setTripPassengers(vector<Request> tripPassengers);

   vector<Request> getTripPassengers();

   void addPassenger(Request request);

   void setTripCost (float cost);

   float getTripCost(); 

   string getTripDetails();

private:
};

#endif /* TRIP_H */

