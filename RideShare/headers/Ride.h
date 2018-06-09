#include<iostream>
#include<cstdlib>
#include "Request.h"
#include<vector>

using namespace std;

#ifndef VEHICLE_H
#define VEHICLE_H

class Ride {
    float longitude;
    float latitude;
    float speed;
    int vehicleID;
    string joinedTime;
    vector<Request> passengerList;
    int capacity;
    float initialWaitingTime;
    int nodeIndex;

public:

    Ride();
    
    Ride(float longitude, float latitude, float speed, int driverID, string joinedTime, int capacity);
    
    void setLocation(float longitude, float latitude);
    
    void setLatitide(float latitude);
    
    void setLongitude(float longitude);
    
    void setDriverID(int driverID);
    
    void setSpeed(float speed);

    void setPassenger(Request request);

    void setPassengerList(vector<Request> requests);
    
    void setJoinedTime(string time);

    void setCapacity(int capacity);

    int getCapacity();
     
    float getLongitude();
    
    float getLatitude();

    int getId();

    void addInititalWaitingTime(float waitingTime);

    float getInititalWaitingTime();

    void reduceInititalWaitingTime(float waitingTime);

    void setNodeIndex(int index);

    int getNodeIndex();

    string getJoinedTime();
    
    vector<Request> getPassengerList();
    
private:
    
};

#endif /* VEHICLE_H */

