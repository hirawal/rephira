#include "../headers/LocationPoint.h"
 
LocationPoint::LocationPoint (float longitude, float latitude, int nodeID) {
    
    this->longitude = longitude;
    this->latitude = latitude;
    this->nodeID = nodeID;
};
    
float LocationPoint::getLongitude() {
    return this->longitude;
}

float LocationPoint::getLatitude() {
    return this->latitude;
}

int LocationPoint::getId() {
    return this->nodeID;
}

