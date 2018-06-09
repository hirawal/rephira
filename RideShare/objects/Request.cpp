#include "../headers/Request.h"

Request::Request(float tripOriginLongitude, 
        float tripOriginLatitude, 
        float tripDestinationLongitude, 
        float tripDestinationLatitude, 
        int requestID,
        string requstedTime) {
    
    this->tripOriginLatitude = tripOriginLatitude;
    this->tripOriginLongitude = tripOriginLongitude;
    this->tripDestinationLatitude = tripDestinationLatitude;
    this->tripDestinationLongitude = tripDestinationLongitude;
    this->requestID = requestID;
    this->requstedTime = requstedTime;
    this->waitingTime = 0;
    this->totalTravelDelay = 0;
    this->idealTravelDelay = 0;
    this->totalServicedTime = 0;
};

float Request::getTripOriginLongitude(){
    return this->tripOriginLongitude;
}

float Request::getTripOriginLatitude() {
    return this->tripOriginLatitude;
}

float Request::getTripDestinationLongitude() {
    return this->tripDestinationLongitude;
}

float Request::getTripDestinationLatitude(){
    return this->tripDestinationLatitude;
}

int Request::getRequestID(){
    return this->requestID;
}

float Request::getWaitingTime() {
    return this->waitingTime;
}

float Request::getTotalTravelDelay(){
    return this->totalTravelDelay;
}

void Request::addToWaitingTime(float waitingTime){
    this->waitingTime = this->waitingTime + waitingTime;
}

void Request::addTotalTravelDelay(float delay){
    this->totalTravelDelay = this->totalTravelDelay = delay;
}

void Request::setTripOriginLongitude(float lat){
    this->tripOriginLatitude = lat;
}
    
void Request::setTripOriginLatitude(float longi){
    this->tripOriginLongitude = longi;
}
    
void Request::setTripDestinationLongitude(float lat){
    this->tripDestinationLatitude = lat;
}
    
void Request::setTripDestinationLatitude(float longi){
    this->tripDestinationLongitude = longi;
}

void Request::setDestinationNodeIndex(int index){
    this->destinationNodeIndex = index;
}

int Request::getDestinationNodeIndex(){
    return this->destinationNodeIndex;
}

void Request::setOriginNodeIndex(int index){
    this->originNodeIndex = index;
}

int Request::getOriginNodeIndex(){
    return this->originNodeIndex;
}

string Request::getRequestedTime(){
    return this->requstedTime;
}
float Request::getidealTravelDelay() {
    return this->idealTravelDelay;
}

void Request::addidealTravelDelay(float idealTime) {
    this->idealTravelDelay = this->idealTravelDelay + idealTime;
}

void Request::addToTotalServicedTime (float assignedTime) {
    this->totalServicedTime = this->totalServicedTime + assignedTime;
}

 float Request::getTotalServicedTime() {
     return this->totalServicedTime;
 }