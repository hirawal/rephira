#include "../headers/Simulator.h"
#include "../headers/Ride.h"
#include "../headers/Constants.h"
#include "../headers/Request.h"
#include "../headers/GeoHasher.h"
#include "../headers/OSRMCaller.h"
#include "../headers/OptimalPathGenerator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include<iostream>
#include<iomanip>
#include<cmath>

using namespace std;
static const double PI = 3.14159265358979323846, earthDiameterMeters = 6371.0 * 2 * 1000;

vector<Ride> Simulator::simulateRide(vector<Ride> vehicleList, map<int, Trip> assignments){
    int size = vehicleList.size();
    for(int t = 0; t < size ;t++) {
        int countCheck = 1;
        for (auto& j: assignments) {
            int vehicleId = j.first;
            Trip trip = j.second;
            if(vehicleId == vehicleList[t].getId() ){
                // new assignments 
                vehicleList.at(t) = moveRide(vehicleList[t], trip);
                countCheck = 0;
            }
        }
        if(countCheck) {
            if(vehicleList[t].getPassengerList().size() > 0) {
                //not new assignment, but has alread assigned requests 
                vehicleList.at(t) = moveRide(vehicleList[t]);
            }
        }
    }
    return vehicleList;

}

Ride Simulator::moveRide(Ride vehicle){
    OptimalPathGenerator pathGenerator;
    vector<Request> newRequests;
    vector<float> pathDetails = pathGenerator.travel(vehicle, newRequests);
    if(pathDetails[0] != -1.0 ){
        double latitude1 = vehicle.getLatitude();
        double longitude1 = vehicle.getLongitude();
        double latitude2 = pathDetails[1];
        double longitude2 = pathDetails[2];
    
        setprecision(10);
        auto angle = CoordinatesToAngle(latitude1, longitude1, latitude2, longitude2);
        auto meters = 0.250;  //with 30kmph
    
        auto coordinate = CoordinateToCoordinate(latitude2, longitude2, angle, meters);

        double newLatitude = coordinate.first;
        double newLongitude = coordinate.second;
        
        GeoHasher geoHasher;
        vector<float> returnValueList = geoHasher.getNearestNodes(newLatitude, newLongitude);

        if(returnValueList.size() > 0) {
            //construct the duration matrix with the locations pioints
            string baseAPICall = "http://localhost:5000/table/v1/driving/" + to_string(longitude2) + "," + to_string(latitude2);

            for (int q = 0; q < returnValueList.size(); q = q+2){
                baseAPICall = baseAPICall + ";" + to_string(returnValueList[q+1]) + "," + to_string(returnValueList[q]);
            }

            OSRMCaller osrmCaller;
            //call OSRM instance to get the local matrix
            vector< vector<float> > matrix = osrmCaller.getMatrix(baseAPICall);
            cout << "[RIDESHARING] \033[0;34m Sim OSRM matrix 3 \033[0m \n";
            int index = 0;
            float minVal = 1000000.0 ; //any large value 
            for(int q = 1; q < matrix.size(); q++) {
                if(matrix[0][q] < minVal) {
                    minVal = matrix[0][q];
                    index = q;
                }
            }
            double distance = geoHasher.distanceEarth(vehicle.getLatitude(), vehicle.getLongitude(), returnValueList[(index-1)*2],returnValueList[(index-1)*2+1] );
            double timeConsumed = distance*1000 / 8.33; // at 30kmph speed
            //set new location
            vehicle.setLatitide(returnValueList[(index-1)*2]);
            vehicle.setLongitude(returnValueList[(index-1)*2+1]);

            for(int u = 0; u < vehicle.getPassengerList().size(); u++) {
                Request request = vehicle.getPassengerList()[u];
                request.addToTotalServicedTime(timeConsumed);
            }
        }

    }
    return vehicle;
}

Ride Simulator::moveRide(Ride vehicle, Trip trip){

    vector<Request> newRequests = trip.getTripPassengers();

    OptimalPathGenerator pathGenerator;
    vector<float> pathDetails = pathGenerator.travel(vehicle, newRequests);
    if(pathDetails[0] != -1.0 ){
        double latitude1 = vehicle.getLatitude();
        double longitude1 = vehicle.getLongitude();
        double latitude2 = pathDetails[1];
        double longitude2 = pathDetails[2];
    
        setprecision(10);
        auto angle = CoordinatesToAngle(latitude1, longitude1, latitude2, longitude2);
        auto meters = 0.250;  //with 10kmph
    
        auto coordinate = CoordinateToCoordinate(latitude2, longitude2, angle, meters);

        double newLatitude = coordinate.first;
        double newLongitude = coordinate.second;
        
        GeoHasher geoHasher;
        vector<float> returnValueList = geoHasher.getNearestNodes(newLatitude, newLongitude);

        if(returnValueList.size() > 0) {
            //construct the duration matrix with the locations pioints
            string baseAPICall = "http://localhost:5000/table/v1/driving/" + to_string(longitude2) + "," + to_string(latitude2);

            for (int q = 0; q < returnValueList.size(); q = q+2){
                baseAPICall = baseAPICall + ";" + to_string(returnValueList[q+1]) + "," + to_string(returnValueList[q]);
            }

            OSRMCaller osrmCaller;
            //call OSRM instance to get the local matrix
            vector< vector<float> > matrix = osrmCaller.getMatrix(baseAPICall);
            cout << "[RIDESHARING] \033[0;34m Sim OSRM matrix 4 \033[0m \n";

            int index = 0;
            float minVal = 1000000.0 ; //any large value 
            for(int q = 1; q < matrix.size(); q++) {
                if(matrix[0][q] < minVal) {
                    minVal = matrix[0][q];
                    index = q;
                }
            }
            double distance = geoHasher.distanceEarth(vehicle.getLatitude(), vehicle.getLongitude(), returnValueList[(index-1)*2],returnValueList[(index-1)*2+1] );
            double timeConsumed  = distance*1000 / 8.33; // at 30kmph speed

            //set new location
            vehicle.setLatitide(returnValueList[(index-1)*2]);
            vehicle.setLongitude(returnValueList[(index-1)*2+1]);

          // cout <<  " *********" << vehicle.getLatitude() <<"," <<vehicle.getLongitude()<< endl;
            //assign new requests
    
            for(int p = 0 ; p < newRequests.size(); p++) {
                if(newRequests[p].getTripOriginLatitude() == vehicle.getLatitude() && newRequests[p].getTripOriginLongitude() == vehicle.getLongitude()){
                    vehicle.setPassenger(newRequests[p]);
                } else {
                    Constants con;
                    if(newRequests[p].getWaitingTime() >= con.getMaxWaitingTime()) {
                        vehicle.setPassenger(newRequests[p]);
                    }
                }
            }

            for(int u = 0; u < vehicle.getPassengerList().size(); u++) {
                Request request = vehicle.getPassengerList()[u];
                request.addToTotalServicedTime(timeConsumed);
            }
        
        }

    }
    return vehicle;
}


double Simulator::degreeToRadian (const double degree) { return (degree * PI / 180); };
double Simulator::radianToDegree (const double radian) { return (radian * 180 / PI); };

double Simulator::CoordinatesToAngle (double latitude1, const double longitude1, double latitude2, const double longitude2) {
    const auto longitudeDifference = degreeToRadian(longitude2 - longitude1);
    latitude1 = degreeToRadian(latitude1);
    latitude2 = degreeToRadian(latitude2);
    
    using namespace std;
    const auto x = (cos(latitude1) * sin(latitude2)) -
    (sin(latitude1) * cos(latitude2) * cos(longitudeDifference));
    const auto y = sin(longitudeDifference) * cos(latitude2);
    
    const auto degree = radianToDegree(atan2(y, x));
    return (degree >= 0)? degree : (degree + 360);
}

double Simulator::CoordinatesToMeters (double latitude1, double longitude1, double latitude2, double longitude2) {
    latitude1 = degreeToRadian(latitude1);
    longitude1 = degreeToRadian(longitude1);
    latitude2 = degreeToRadian(latitude2);
    longitude2 = degreeToRadian(longitude2);
    
    auto x = sin((latitude2 - latitude1) / 2), y = sin((longitude2 - longitude1) / 2);
    #if 1
        return earthDiameterMeters * asin(sqrt((x * x) + (cos(latitude1) * cos(latitude2) * y * y)));
    #else
        auto value = (x * x) + (cos(latitude1) * cos(latitude2) * y * y);
        return earthDiameterMeters * atan2(sqrt(value), sqrt(1 - value));
    #endif
}

pair<double,double> Simulator::CoordinateToCoordinate (double latitude,double longitude,double angle,double meters) {
    latitude = degreeToRadian(latitude);
    longitude = degreeToRadian(longitude);
    angle = degreeToRadian(angle);
    meters *= 2 / earthDiameterMeters;
    
    using namespace std;
    pair<double,double> coordinate;

    coordinate.first = asin((sin(latitude) * cos(meters)) + (cos(latitude) * sin(meters) * cos(angle)));

    coordinate.second = longitude + atan2((sin(angle) * sin(meters) * cos(latitude)), cos(meters) - (sin(latitude) * sin(coordinate.first)));

    coordinate.first = radianToDegree(coordinate.first);
    coordinate.second = radianToDegree(coordinate.second);
    
    return coordinate;
}
