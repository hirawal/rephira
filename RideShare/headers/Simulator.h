#include <map>
#include <vector>
#include "Request.h"
#include "Trip.h"
#include "Ride.h"
#include <iostream>
#include <iomanip>
#include <cmath>

#ifndef SIMULATOR_H
#define SIMULATOR_H

class Simulator {
public:
    vector<Ride> simulateRide(vector<Ride> list_Of_Taxis, map<int, Trip> assignments);

private:

    Ride moveRide(Ride vehicle);
    Ride moveRide(Ride vehicle, Trip trip);
    pair<double,double> CoordinateToCoordinate (double latitude,double longitude,double angle,double meters);
    double CoordinatesToMeters (double latitude1, double longitude1, double latitude2, double longitude2);
    double CoordinatesToAngle (double latitude1, const double longitude1, double latitude2, const double longitude2);
    double degreeToRadian (const double degree);
    double radianToDegree (const double radian);

};

#endif /* SIMULATOR_H */

