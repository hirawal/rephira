#include<iostream>
#include<cstdlib>
using namespace std;

#ifndef PASSENGER_H
#define PASSENGER_H

class Request {
    float tripOriginLongitude;
    float tripOriginLatitude;
    float tripDestinationLongitude;
    float tripDestinationLatitude;
    int requestID;
    string requstedTime;
    float waitingTime;
    float totalTravelDelay;
    int originNodeIndex;
    int destinationNodeIndex;
    float idealTravelDelay;
    float totalServicedTime;
    
public:
    Request(float tripOriginLongitude, float tripOriginLatitude, 
            float tripDestinationLongitude, float tripDestinationLatitude, 
            int requestID, string requstedTime );
    
    float getTripOriginLongitude();
    
    float getTripOriginLatitude();
    
    float getTripDestinationLongitude();
    
    float getTripDestinationLatitude();
    
    int getRequestID();

    float getWaitingTime();

    float getTotalTravelDelay();

    void addTotalTravelDelay(float travelTime);

    float getidealTravelDelay();

    void addidealTravelDelay(float idealTime);

    void addToWaitingTime(float waitingTime);

    void setTripOriginLongitude(float lat);
    
    void setTripOriginLatitude(float longi);
    
    void setTripDestinationLongitude(float lat);
    
    void setTripDestinationLatitude(float longi);
    
    void setOriginNodeIndex(int index);

    int getOriginNodeIndex();

    void setDestinationNodeIndex(int index);

    int getDestinationNodeIndex();

    string getRequestedTime();

    void addToTotalServicedTime (float assignedTime);

    float getTotalServicedTime();

private:

};

#endif /* PASSENGER_H */

