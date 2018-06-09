#include<iostream>
#include<cstdlib>

using namespace std;

#ifndef LOCATION_POINT_H
#define LOCATION_POINT_H

class LocationPoint {
    float longitude;
    float latitude;
    float nodeID;

public:
    LocationPoint (float longitude, float latitude, int nodeID);

    float getLongitude();
    
    float getLatitude();

    int getId();

private:
    
};

#endif /* LOCATION_POINT_H */

