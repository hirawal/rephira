#include <vector>
#include "Ride.h"
#include "Request.h"
#include "../headers/LocationPoint.h"
#include <map>
#include "Trip.h"

#ifndef PATHGEN_H
#define PATHGEN_H

using namespace std;

class OptimalPathGenerator{

    vector<vector<float> > routeMatrix;;

public:
    void initPlanner();
    vector<float> travel(Ride vehicle, vector<Request> list_Of_Requests);
    map<int, vector<Trip> > getTravelCosts(vector<Ride> unassignedRides, vector<Request> unassignedRequests);

private:
    void permute(string select, string remain, vector<string> *permutationsList);
    vector<float> getFinalPath(vector< vector<float> > matrix, vector<string> validPermutations,
                 vector<Request> list_Of_Requests, Ride vehicle, vector<LocationPoint> pathPointList, int totalPassengers);
};

#endif /* PATHGEN_H */

