#include <vector>
#include "Ride.h"
#include "Request.h"

#ifndef GEOHASHER_H
#define GEOHASHER_H

using namespace std;

class GeoHasher{
  
public:
    void initHasher();
    vector<float> getNearestNode(double latitude, double longitude);
    vector<float> getNearestNodes(double latitude, double longitude);
    double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d);

private:
    void getNodeData();
    void getHashData();
    double rad2deg(double rad);
    double deg2rad(double deg);
};

#endif /* GEOHASHER_H */

