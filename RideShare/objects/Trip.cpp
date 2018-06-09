#include "../headers/Trip.h"

    
void Trip::setTripPassengers(vector<Request> tripPassengers){
    this->tripPassengers = tripPassengers;
}

vector<Request> Trip::getTripPassengers(){
        return this->tripPassengers;
}

void Trip::addPassenger(Request request){
    this->tripPassengers.push_back(request);
}

void Trip::setTripCost (float cost){
    this->tripCost = cost;
}

float Trip::getTripCost(){
   return this->tripCost;
}

string Trip::getTripDetails(){
    string details;
    for(int i = 0; i< this->tripPassengers.size(); i++) {
        details = details + to_string(tripPassengers[i].getRequestID()) + "-";
    }

    return details.substr(0,details.size()-1);
}