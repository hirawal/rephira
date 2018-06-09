#include "../headers/Ride.h"
 
Ride::Ride(){
    //nothing
}

Ride::Ride (float longitude, float latitude, float speed, 
        int vehicleID, string joinedTime, int capacity) {
    
    this->longitude = longitude;
    this->latitude = latitude;
    this->speed = speed;
    this->vehicleID = vehicleID;
    this->joinedTime = joinedTime;
    this->capacity = capacity;
    this->initialWaitingTime = 0;
};
    
void Ride::setLocation (float longitude, float latitude) {
    this->longitude = longitude;
    this->latitude = latitude;
}

void Ride::setLatitide(float latitude){
    this->latitude = latitude;
}
    
void Ride::setLongitude(float longitude){
    this->longitude = longitude;
}
    
void Ride::setDriverID(int driverID){
    this->vehicleID = driverID;
}
    
void Ride::setSpeed(float speed){
    this->speed = speed;
}
 
void Ride::setJoinedTime(string time){
    this->joinedTime = time;
}
 
float Ride::getLongitude() {
    return this->longitude;
}

float Ride::getLatitude() {
    return this->latitude;
}

void Ride::setPassenger(Request request){
    this->passengerList.push_back(request);
}

void Ride::setPassengerList(vector<Request> requests){
    this->passengerList = requests;
}

void Ride::setCapacity(int capacity){
    this->capacity = capacity;
}

int Ride::getCapacity(){
    return this->capacity;
}

int Ride::getId() {
    return this->vehicleID;
}

void Ride::addInititalWaitingTime(float waitingTime) {
    this->initialWaitingTime = this->initialWaitingTime + waitingTime;
}

float Ride::getInititalWaitingTime(){
    return this->initialWaitingTime;
}

void Ride::reduceInititalWaitingTime(float waitingTime){
    this->initialWaitingTime = this->initialWaitingTime - waitingTime;
}

void Ride::setNodeIndex(int index){
    this->nodeIndex = index;
}

int Ride::getNodeIndex(){
    return this->nodeIndex;
}

string Ride::getJoinedTime() {
    return this->joinedTime;
}

vector<Request> Ride::getPassengerList(){
    return this->passengerList;
}
