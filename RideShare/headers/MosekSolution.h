#include <map>
#include "Request.h"
#include "Trip.h"
#include "Ride.h"

#ifndef ILPSOLVER_H
#define ILPSOLVER_H

class MosekSolution {
public:
    vector<int> solve(map<int, vector<Trip> > list_Of_Trips, vector<Request> list_Of_Requests);
    vector<int> solveForRebalancing(map<int, vector<Trip> > list_Of_Trips, vector<Request> list_Of_Requests, vector<Ride> unassignedRides) ;

private:

};

#endif /* ILPSOLVER_H */

