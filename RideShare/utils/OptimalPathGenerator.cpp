#include "../headers/OptimalPathGenerator.h"
#include "../headers/Ride.h"
#include "../headers/Request.h"
#include "../headers/Constants.h"
#include "../headers/LocationPoint.h"
#include "../headers/OSRMCaller.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator> 
#include <stdio.h>
#include <curl/curl.h>
#include <map>
#include "../headers/Trip.h"

#define LOOP_COUNT 70;
#define TOTAL_NODE_COUNT 24559;
#define ITERATION_COUNT 350; // (24559/70)

//initialize the route matrix
void OptimalPathGenerator::initPlanner(){
    int blockCount = 0;
    int localBlockCount = 0;
    int lineCount = 0;

    ifstream file("data/matrix.txt");
    string str;
    int c =0;

    while (getline(file, str))
    {
        istringstream ss(str);
        string token;
        int elementCount = 0;
        c++;
        while(getline(ss, token, ',')) {

            if (token.substr(0,1) == "[") {
                //routeMatrix[blockCount*70 + lineCount].insert(routeMatrix[blockCount*70 + lineCount].begin() + elementCount, stof(token.substr(1, token.size())));
            
            } else if (token.substr(token.size()-1,token.size()) == "]"){
                //routeMatrix[blockCount*70 + lineCount].insert(routeMatrix[blockCount*70 + lineCount].begin() + elementCount, stof(token.substr(0, token.size()-1)));
         
            } else {
               //routeMatrix[blockCount*70 + lineCount].insert(routeMatrix[blockCount*70 + lineCount].begin() + elementCount, stof(token.substr(1, token.size())));
            } 
            elementCount++;
        }
        
        localBlockCount++;
        lineCount++;
        if(lineCount == 69) {
            lineCount = 0;
        }
        if(localBlockCount == 70*351 - 1) {
            blockCount++;
            localBlockCount = 0;
        }
    }
}

//the implementation of Travel() function
vector<float> OptimalPathGenerator::travel(Ride vehicle, vector<Request> list_Of_Requests){

    try {
    vector<float> finalPath;
    
    vector<LocationPoint> pathPointList;
    vector<int> pathPointValidationList;

    vector<Request> currentPassengerList = vehicle.getPassengerList();

    //check for vehcile capacity constrint vialation
    int totalPassengers = currentPassengerList.size() + list_Of_Requests.size();

    if(totalPassengers > vehicle.getCapacity() || totalPassengers == 0){
        finalPath.push_back(-1.0);
        //retrun null route if can not be serverd
        return finalPath;
    }

    for (int i = 0; i < currentPassengerList.size(); i++) {
        LocationPoint locationDestinationPoint (currentPassengerList[i].getTripDestinationLongitude(), 
                        currentPassengerList[i].getTripDestinationLatitude(), currentPassengerList[i].getDestinationNodeIndex());
        pathPointList.push_back(locationDestinationPoint);
        //indication that these are current passenagers related location points
        pathPointValidationList.push_back(2);
    }
    
    for (int i = 0; i < list_Of_Requests.size(); i++) {

        LocationPoint locationOriginPoint (list_Of_Requests[i].getTripOriginLongitude(), 
                        list_Of_Requests[i].getTripOriginLatitude(), list_Of_Requests[i].getOriginNodeIndex());
        LocationPoint locationDestinationPoint (list_Of_Requests[i].getTripDestinationLongitude(), 
                        list_Of_Requests[i].getTripDestinationLatitude(), list_Of_Requests[i].getDestinationNodeIndex());
        pathPointList.push_back(locationOriginPoint);
        pathPointList.push_back(locationDestinationPoint);
        //indication that these are new passnagers related location points - origin location
        pathPointValidationList.push_back(0);
        //indication that these are new passnagers related location points - destination location
        pathPointValidationList.push_back(1);
    }

    //get the possible permutations
    int sizeOfLocationPoints = currentPassengerList.size() + list_Of_Requests.size()*2;

    string anagrama = "";
    if(totalPassengers <= 4){
        for (int i = 0 ; i< sizeOfLocationPoints ; i++){
            stringstream numberSS;
            numberSS << i;
            string temp  = numberSS.str();
            anagrama = anagrama + temp;
        }
    } else {
        anagrama = "01234567";
    }

    vector<string> *v  = new vector<string>(0);

    vector<string> &permutationsList = *v; 
    sort(anagrama.begin(), anagrama.end());
    permute("", anagrama, v);


    // get only the valid permutations (ex : drop off should come before pickup)
    vector<string> validPermutations;

    for(int y = 0; y < permutationsList.size(); y++) {
        int overallCheck = 1;
        string permutation = permutationsList[y];
        for(int t = 0; t < permutation.size(); t++) {
            int temp  = stoi(permutation.substr(t,1));
            if(pathPointValidationList[temp] == 1) {
                int check = 0;
                for(int z = 0; z < t; z++) {
                    if (permutation.substr(z,1) == to_string(temp - 1)) {
                        check = 1;
                        break;
                    }
                }
                if(!check) {
                    overallCheck = 0;
                    break;
                }
            }
        }
        if(overallCheck) {
            validPermutations.push_back(permutation);
        }
    }

    //construct the duration matrix with the locations pioints

    string baseAPICall = "http://localhost:5000/table/v1/driving/" + to_string(vehicle.getLongitude()) + "," + to_string(vehicle.getLatitude());

    for (int q = 0; q < pathPointList.size(); q ++){
        baseAPICall = baseAPICall + ";" + to_string(pathPointList[q].getLongitude()) + "," + to_string(pathPointList[q].getLatitude());
    }
    //cout << "[travel]" << baseAPICall << endl;
    OSRMCaller osrmCaller;
    //call OSRM instance to get the local matrix
    vector< vector<float> > matrix = osrmCaller.getMatrix(baseAPICall);

    finalPath = getFinalPath(matrix,validPermutations, list_Of_Requests, vehicle, pathPointList, totalPassengers);
    permutationsList.clear();
    v->clear();
    return finalPath;
} catch (...) {
    vector<float> finalPath;
    finalPath.push_back(-1.0);
    cout << "[travel] Caught red-handed";
    return finalPath;
}
}

void OptimalPathGenerator::permute(string select, string remain, vector<string> *v){
    vector<string> &vr = *v; 
    if(remain == ""){  
        vr.push_back(select);
        return;
    }
    for(int i=0;remain[i];++i){
        string wk(remain);
        permute(select + remain[i], wk.erase(i, 1), v);
    }
}

//find the route with the best total travel delay and does not violate the individual request constraints
vector<float> OptimalPathGenerator::getFinalPath(vector< vector<float> > matrix, vector<string> validPermutations, 
                                                vector<Request> list_Of_Requests, Ride vehicle, vector<LocationPoint> pathPointList, int totalPassengers ) {
    vector<float> finalPath;
    vector<float> totalTravelDelays;
    Constants con;

    for(int i = 0; i < validPermutations.size();i++) {
        int check = 1;
        string permutation = validPermutations[i];
        int count = 0;
        float totalTravelDelayForRoute = 0;

        vector<Request> currentPassengers = vehicle.getPassengerList();


        //in-vehicle passengers constraint check 
        for(int j = 0; j < currentPassengers.size(); j++){
            Request inVehcileRequest = currentPassengers[j];
            float assignedTime = inVehcileRequest.getTotalServicedTime();
            vector<int> tempIndex;

            for(int k = 0; k < permutation.size(); k++) {
                if(permutation.substr(k, 1) == to_string(count)) {
                    break;
                } else {
                    tempIndex.push_back(stoi(permutation.substr(k,1)));
                }
            }
            int y = 0;
            for(y = 0; y < tempIndex.size(); y ++) {
                if(y == 0){
                    assignedTime = assignedTime + matrix[0][tempIndex[y]+1];
                } else {
                    assignedTime = assignedTime + matrix[tempIndex[y-1]+1][tempIndex[y]+1];
                }
            }

            if(tempIndex.size() > 0) {
                assignedTime = assignedTime + matrix[tempIndex[y-1]+1][count+1];
            }
            
            // break if total travel delay time exceed the bound
            if (assignedTime - inVehcileRequest.getidealTravelDelay() > con.getMaxDelay()){
                check = 0 ;
                break;
            } else{
                totalTravelDelayForRoute = totalTravelDelayForRoute + assignedTime - inVehcileRequest.getidealTravelDelay() ;
                //cout << "\r\n[getFinalPath] totalTravelDelay For Route is " << totalTravelDelayForRoute << endl;
            }
            count++;
        }

        if(check) {
        //new passenger constraint check
        for(int j = 0; j < list_Of_Requests.size(); j++){
            Request newPassanger = list_Of_Requests[j];
            //check waiting time constraint
            vector<int> tempIndex;
            for(int k = 0; k < permutation.size(); k++) {
                if(permutation.substr(k, 1) == to_string(count)) {
                    break;
                } else {
                    //cout << "\r\n[getFinalPath] new passenger push" << endl;
                    tempIndex.push_back(stoi(permutation.substr(k,1)));
                }
            }
                    
            float waitingTime = 0;
            int y = 0;
            for(y = 0; y < tempIndex.size(); y ++) {
                if(y == 0){
                    waitingTime = newPassanger.getWaitingTime() + waitingTime + matrix[0][tempIndex[y]+1];
                } else {
                    waitingTime = waitingTime + matrix[tempIndex[y-1]+1][tempIndex[y]+1];
                }
            }
            if(y > 0){
                waitingTime = waitingTime + matrix[tempIndex[y-1]+1][count+1];
            } else {
                waitingTime = waitingTime + matrix[0][count+1];
            }
            //cout << "\r\n[getFinalPath] waitingTime is " << waitingTime << endl;
            // break if waiting time exceed the bound
            if (waitingTime > con.getMaxWaitingTime()){
                check = 0 ;
                break;
            }

            tempIndex.clear();

            //set ideal travel time
            newPassanger.addidealTravelDelay(matrix[count+1][count+2]);
            //check total travel delay constraint
            vector<int> tempIndexForDelay;
            for(int y = tempIndexForDelay.size()+1; y < permutation.size(); y ++) {
                if(permutation.substr(y,1) == to_string(count+1)){
                    break;
                } else {
                    tempIndexForDelay.push_back(stoi(permutation.substr(y,1)));
                }
            }

            float travelDelay = waitingTime;
            y = 0;
            for(y = 0; y < tempIndexForDelay.size(); y ++) {
                if(y == 0){
                    travelDelay = travelDelay + (matrix[count+1][tempIndexForDelay[y]+1]);
                } else {
                    travelDelay = travelDelay + (matrix[tempIndexForDelay[y-1]+1][tempIndexForDelay[y]+1]);
                }
            }
            if(y > 0) {
                travelDelay = travelDelay + matrix[tempIndexForDelay[y-1]+1][count+1+1];
            } else {
                travelDelay = travelDelay + newPassanger.getidealTravelDelay();
            }

            // break if total travel delay time exceed the bound
            if (travelDelay - newPassanger.getidealTravelDelay() > con.getMaxDelay()){
                check = 0 ;
                break;
            } else{
                totalTravelDelayForRoute = totalTravelDelayForRoute + travelDelay - newPassanger.getidealTravelDelay() ;
            }
            tempIndexForDelay.clear();
        }
        //cout << "\r\n[getFinalPath] totalTravelDelayForRoute is --> " << totalTravelDelayForRoute << endl;
        } 
        if(!check) {
            totalTravelDelays.push_back(-1.0);
        } else {
            totalTravelDelays.push_back(totalTravelDelayForRoute);
        }
    }

    int minIndex = -1;
    int checkCount = 0;
    for (int i = 0; i < totalTravelDelays.size(); i++){
        if(totalTravelDelays[i] != -1.0 ) {
            checkCount++;
            if(checkCount == 1){
                minIndex = i;
            } else {
                if (totalTravelDelays[minIndex] > totalTravelDelays[i]) {
                minIndex = i;
            }
         }
            
        } else {

        }
    }
    //cout << "\r\n[getFinalPath] minIndex is " << minIndex << endl;
    //no valid route for the given request
    if(minIndex == -1){
        finalPath.push_back(-1.0);
        return finalPath;
    }
    string rightPermutation = validPermutations[minIndex];

    if(totalPassengers <= 4){ 
        finalPath.push_back(totalTravelDelays[minIndex]);

        for(int p = 0; p < rightPermutation.size(); p++){
            int nextPoint = stoi(rightPermutation.substr(p, 1));
            finalPath.push_back(pathPointList[nextPoint].getLatitude());
            finalPath.push_back(pathPointList[nextPoint].getLongitude());
        }
    
        return finalPath;

    } else {
        //accomodate requests with more than 4 passengers

        float totalTravelDelayForExtendedRoute = totalTravelDelays[minIndex];

        int start = vehicle.getPassengerList().size() + (4 - vehicle.getPassengerList().size()) * 2;
        int passngerIndex = list_Of_Requests.size() - (list_Of_Requests.size() - (4 - vehicle.getPassengerList().size())) ;
        string newPermutation = rightPermutation;

        int check = 1;
        for (int q = start; q < list_Of_Requests.size(); q = q + 2) {
            newPermutation = newPermutation + to_string(q) + to_string(q+1);
            Request newPassanger = list_Of_Requests[passngerIndex];
            //check for waiting time constraint
            vector<int> tempIndex;
                for(int k = 0; k < newPermutation.size(); k++) {
                    if(newPermutation.substr(k, 1) == to_string(q)) {
                        break;
                    } else {
                        tempIndex.push_back(stoi(newPermutation.substr(k,1)));
                    }
                }
                    
                float waitingTime = 0;
                int y = 0;
                for(y = 0; y < tempIndex.size(); y ++) {
                    if(y == 0){
                        waitingTime = newPassanger.getWaitingTime() + waitingTime + matrix[0][tempIndex[y]+1];
                    } else {
                        waitingTime = waitingTime + matrix[tempIndex[y-1]+1][tempIndex[y]+1];
                    }
                }
                if(y > 0){
                    waitingTime = waitingTime + matrix[tempIndex[y-1]+1][q+1];
                } else {
                    waitingTime = waitingTime + matrix[0][q+1];
                }

                // break if waiting time exceed the bound
                if (waitingTime > con.getMaxWaitingTime()){
                    check = 0 ;
                    break;
                }

                tempIndex.clear();

                //set ideal travel time
                newPassanger.addidealTravelDelay(matrix[q+1][q+2]);
                //check total travel delay constraint
                vector<int> tempIndexForDelay;
                for(int y = tempIndexForDelay.size()+1; y < newPermutation.size(); y ++) {
                    if(newPermutation.substr(y,1) == to_string(q+1)){
                        break;
                    } else {
                        tempIndexForDelay.push_back(stoi(newPermutation.substr(y,1)));
                    }
                }

                float travelDelay = waitingTime;
                y = 0;
                for(y = 0; y < tempIndexForDelay.size(); y ++) {
                    if(y == 0){
                        travelDelay = travelDelay + (matrix[q+1][tempIndexForDelay[y]+1]);
                    } else {
                        travelDelay = travelDelay + (matrix[tempIndexForDelay[y-1]+1][tempIndexForDelay[y]+1]);
                    }
                }
                if(y > 0) {
                    travelDelay = travelDelay + matrix[tempIndexForDelay[y-1]+1][q+1+1];
                } else {
                    travelDelay = travelDelay + newPassanger.getidealTravelDelay();
                }

                // break if total travel delay time exceed the bound
                if (travelDelay - newPassanger.getidealTravelDelay() > con.getMaxDelay()){
                    check = 0 ;
                    break;
                } else{
                    totalTravelDelayForExtendedRoute = totalTravelDelayForExtendedRoute + travelDelay - newPassanger.getidealTravelDelay() ;
                }
                tempIndexForDelay.clear();
            }

        if(!check)  {
            finalPath.push_back(-1.0);

            return finalPath;
        }

        finalPath.push_back(totalTravelDelayForExtendedRoute);

        for(int p = 0; p < newPermutation.size(); p++){
            int nextPoint = stoi(rightPermutation.substr(p, 1));
            finalPath.push_back(pathPointList[nextPoint].getLatitude());
            finalPath.push_back(pathPointList[nextPoint].getLongitude());
        }
        return finalPath;
    }
    finalPath.push_back(-1.0);
    return finalPath;
    
}


//find the route with the best total travel delay and does not violate the individual request constraints
map<int, vector<Trip> > OptimalPathGenerator::getTravelCosts(vector<Ride> unassignedRides, vector<Request> unassignedRequests) {

    map<int, vector<Trip> > list_Of_Trips;
    //cout << "[SETHINFO] \033[0;32m  getTravelCosts....." << endl;
    for(int i = 0; i < unassignedRides.size(); i++) {
            Ride vehicle = unassignedRides[i];
            vector<Trip> trips;
            
            //construct the duration matrix with the locations pioints
                string baseAPICall = "http://localhost:5000/table/v1/driving/" + to_string(vehicle.getLongitude()) + "," + to_string(vehicle.getLatitude());

            for (int q = 0; q < unassignedRequests.size(); q ++){
                    baseAPICall = baseAPICall + ";" + to_string(unassignedRequests[q].getTripOriginLongitude()) 
                                              + "," + to_string(unassignedRequests[q].getTripOriginLatitude());
            }
            OSRMCaller osrmCaller;

            //call OSRM instance to get the local matrix
            vector< vector<float> > matrix = osrmCaller.getMatrix(baseAPICall);

            //cout << "[SETHINFO] \033[0;32m  matrix ......" << endl;
            for(int y = 0; y < unassignedRequests.size(); y++) {
                Request request = unassignedRequests[y];
                Trip trip;
                trip.addPassenger(request);
                float cost = matrix[0][y+1];
                trip.setTripCost(cost);
                trips.push_back(trip);
            }
            list_Of_Trips[vehicle.getId()] = trips;
            trips.clear();
    }

    //cout << "[SETHINFO] \033[0;32m  Returning from getTravelCosts....." << endl;
    return list_Of_Trips;

}
