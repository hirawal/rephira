#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pthread.h>
#include "headers/Ride.h"
#include "headers/OptimalPathGenerator.h"
#include "headers/Request.h"
#include "headers/Constants.h"
#include "headers/DataReader.h"
#include "headers/GeoHasher.h"
#include "headers/BufferManager.h"
#include "headers/Simulator.h"
#include "headers/MosekSolution.h"
#include "headers/Trip.h"
#include <vector>
#include <pthread.h>
#include <map>
#include <mutex>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>

#define INTERVAL 30.0


using namespace std;

/* struct for argument parsing for RV graph threads */
struct t_Data {
    long thr_ID;
    long no_Of_Thr;
    vector<Ride> list_Of_Taxis;
    vector<Request> list_Of_Requests;
};

/* struct for argument parsing for RTV graph threads */
struct t_Data_RTV {
    long thr_ID;
    long no_Of_Thr;
    map<int, vector<Request> > list_Of_VR_Edges;
    map<int, vector<Request> > list_Of_RR_Edges;
    vector<Ride> list_Of_Taxis;
};

/* map to store possible trips per vehicle */
map<int, vector<Trip> > list_Of_Trips;

/* vector to store v-r edges for vehicle V */
map<int, vector<Request> > list_Of_VR_Edges;

/* vector to store r-r edges for request r */
map<int, vector<Request> > list_Of_RR_Edges;

mutex mtx;


/* Function to create RTV grapgh */
void *Make_RTV_Graph(void *thread_obj)
{
   
    struct t_Data_RTV *thread_object;
    thread_object = (struct t_Data_RTV *) thread_obj;

    long thr_ID = (long)thread_object->thr_ID;
    long t_Count = (long)thread_object->no_Of_Thr;

    map<int, vector<Request> > list_Of_VR_Edges = (map<int, vector<Request> >)thread_object->list_Of_VR_Edges;

    map<int, vector<Request> > list_Of_RR_Edges = (map<int, vector<Request> >)thread_object->list_Of_RR_Edges;

    vector<Ride> list_Of_Taxis = (vector<Ride>)thread_object->list_Of_Taxis;

    int no_Of_Taxis = list_Of_VR_Edges.size();
    float countCheck = (no_Of_Taxis - (thr_ID)*(no_Of_Taxis/t_Count))/float(no_Of_Taxis/t_Count);
    //cout << "\r\n[RIDESHARING] \033[0;30m Seth no_of_Taxis is " << no_Of_Taxis << "countCheck is " << countCheck << endl;

    // check for traversing all elements of the vector
    OptimalPathGenerator pathGenerator;
    if(countCheck >= 1.0) {
        int count = ceil((thr_ID)*(no_Of_Taxis/(float)t_Count));
        for(int i = ceil((thr_ID)*(no_Of_Taxis/(float)t_Count));  i < ceil((thr_ID+1)*(no_Of_Taxis/(float)t_Count)); i++) {
            Ride vehicle;
            vector<Trip> possible_Trips;
            vector<Request> tempList;
            int taxi_ID = 0;

            int localCount = 0;
            //for(map<int, vector<Request>>::iterator x=list_Of_VR_Edges.begin(); x!=list_Of_VR_Edges.end(); ++x) {
            for (auto& x: list_Of_VR_Edges) {
                if(localCount < i) {   //count Seth
                    localCount++;
                    continue;
                } else {
                    taxi_ID = x.first;
                    tempList = x.second;
                    //cout << "-->taxi_ID is " << taxi_ID << endl;
                    //cout << "-->tempList  is " << tempList.size() << endl;
                    break;
                }
            }

            /* get the taxi related */
            for (int q = 0; q < list_Of_Taxis.size(); q++){
                if(list_Of_Taxis[q].getId() == taxi_ID) {
                    vehicle = list_Of_Taxis[q];
                    //cout << "--> related vehicle is " << vehicle.getId() << endl;
                    break;
                }
            }

            int start = 0;
            int end = possible_Trips.size();
            for (int  k = 1; k <= vehicle.getCapacity(); k++ ) {
                //Trip of size 1
                if(k == 1) { //cout << "k=1 " << endl;
                    for(int t =0; t< tempList.size();t++) {
                        vector<Request> list_Of_Temporary_Requests;
                        list_Of_Temporary_Requests.push_back(tempList[t]);
                        vector<float> pathDetails = pathGenerator.travel(vehicle, list_Of_Temporary_Requests);
                        //cout << "1 " << pathDetails[0] << endl;
                        if (pathDetails[0] != -1.0) {
                                Trip trip;
                                trip.addPassenger(tempList[t]);
                                trip.setTripCost(pathDetails[0]);
                                possible_Trips.push_back(trip);
                                //cout << "k=1 trip cost is " << trip.getTripCost() << endl;
                        }
                        list_Of_Temporary_Requests.clear();
                    }
                    end = possible_Trips.size();
                } //Trip size of 2 
                else if(k == 2){
                    for(int y = start; y < end; y++) {
                        for(int p = y+1; p < end; p++) {
                            vector<Request> list_Of_Temporary_Requests;
                            list_Of_Temporary_Requests.push_back(tempList[y]);
                            list_Of_Temporary_Requests.push_back(tempList[p]);
                            vector<float> pathDetails = pathGenerator.travel(vehicle, list_Of_Temporary_Requests);
                            if (pathDetails[0] != -1.0) {
                                Trip trip;
                                trip.addPassenger(tempList[y]);
                                trip.addPassenger(tempList[p]);
                                trip.setTripCost(pathDetails[0]);
                                possible_Trips.push_back(trip);
                            }
                            list_Of_Temporary_Requests.clear();
                        }
                    }
                    start = end;
                    end = possible_Trips.size();
                } //Trip size of 3 or more
                else {
                    for(int y = start; y < end; y++) {
                        for(int p = y + 1; p < end; p++) {
                            vector<Request> tempCombinedTrips;
                            Trip out_Trips = possible_Trips[y];
                            Trip in_Trips = possible_Trips[p];
                            tempCombinedTrips = out_Trips.getTripPassengers();
                            int sizet = tempCombinedTrips.size();

                            /* create next level of trip size */
                            for(int u = 0; u < in_Trips.getTripPassengers().size(); u++){
                                int checkVal = 1;
                                for(int r = 0; r < sizet; r++){
                                    /* Trip is already present in the list or not */
                                    if(tempCombinedTrips[r].getRequestID() == in_Trips.getTripPassengers()[u].getRequestID()) {
                                        checkVal = 0;
                                        break;
                                    }
                                }
                                if (checkVal) {
                                     tempCombinedTrips.push_back(in_Trips.getTripPassengers()[u]);
                                }
                            }
                            /* only trips with considered capacity is allowed */
                            if (tempCombinedTrips.size() == k){
                                vector<Request> temp;
                                for (int a = 0; a < k; a++) {
                                    for (int b = 0; b < k; b++) {
                                        if (b != a) {
                                            temp.push_back(tempCombinedTrips[b]);
                                        }
                                    }  
                                }
                                int topCheckCount = 0;
                                for(int l = 0; l < temp.size(); l = l +(k-1)) { 
                                   int mainCheckCount = 0;
                                   for(int j = start; j < end; j++) { 
                                       vector<Request> currentTripRequest = possible_Trips[j].getTripPassengers();
                                       int checkCount = 0;
                                       for (int w = 0; w < currentTripRequest.size(); w++) {
                                            for (int e = l; e < l + k-1 ; e++) {
                                                if(temp[e].getRequestID() == currentTripRequest[w].getRequestID()){
                                                    checkCount++;
                                                }
                                            }
                                        }
                                        if(checkCount == k-1) {
                                            mainCheckCount = 1;
                                            break;
                                        }
                                    }
                                    if (!mainCheckCount) {
                                        topCheckCount = 1;
                                        break;
                                    }
                               }
                               if(!topCheckCount) {
                                    vector<Request> list_Of_Temporary_Requests;
                                    for (int d = 0; d < tempCombinedTrips.size(); d++) {
                                        list_Of_Temporary_Requests.push_back(tempCombinedTrips[d]);
                                    }
                                    //cehck for duplicates 
                                    int duplicateCheck = 0;
                                    for (int z = end; z < possible_Trips.size(); z++) {
                                        vector<Request> currentRequests = possible_Trips[z].getTripPassengers();
                                        int trunCheck = 0;
                                        for (int d = 0; d < list_Of_Temporary_Requests.size(); d++) {
                                            int locCheck = 0;
                                            for (int x = 0; x < currentRequests.size(); x++) {
                                                if(list_Of_Temporary_Requests[d].getRequestID() == currentRequests[x].getRequestID()){
                                                    locCheck = 1;
                                                    trunCheck++;
                                                    break;
                                                } 
                                            }
                                            if(!locCheck) {
                                                break;
                                            }
                                        }
                                        if(trunCheck == k) {
                                            duplicateCheck = 1;
                                        }
                                    }
                                    if (!duplicateCheck) {
                                        vector<float> pathDetails = pathGenerator.travel(vehicle, list_Of_Temporary_Requests);
                                        //cout << "3 " << pathDetails[0] << endl;
                                        if (pathDetails[0] != -1.0) {
                                            Trip trip;
                                            for (int d =0; d < tempCombinedTrips.size(); d++) {
                                                trip.addPassenger(tempCombinedTrips[d]);
                                            }
                                            trip.setTripCost(pathDetails[0]);
                                            possible_Trips.push_back(trip);
                                            //cout << "k>2 trip cost is " << trip.getTripCost() << endl;
                                        }
                                        list_Of_Temporary_Requests.clear();
                                    }
                               }

                            }
                        }
                    }
                    start = end;
                    end = possible_Trips.size();
                }
            }
            mtx.lock();
            list_Of_Trips[vehicle.getId()] = possible_Trips;
            cout << "[RIDESHARING] \033[0;35m 1. Possible Trip Size for Ride " << vehicle.getId() << "  is " << possible_Trips.size() << "" << endl;
            possible_Trips.clear();
            mtx.unlock();
        }
    } 
    else {
        int count = ceil((thr_ID)*(no_Of_Taxis/(float)t_Count));
        for(int i = ceil((thr_ID)*(no_Of_Taxis/(float)t_Count));  i < no_Of_Taxis; i++) {
             Ride vehicle;
            vector<Trip> possible_Trips;
            vector<Request> tempList;
            int taxi_ID = 0;
            int localCount = 0;
            for (auto& x: list_Of_VR_Edges) {
                if(localCount < i) {    //count
                    localCount++;
                    continue;
                } else {
                    taxi_ID = x.first;
                    tempList = x.second;
                    break;
                } 
            }
            //get the vehicle related 
            for (int q = 0; q < list_Of_Taxis.size(); q++){
                if(list_Of_Taxis[q].getId() == taxi_ID) {
                    vehicle = list_Of_Taxis[q];
                    break;
                }
            }

            int start = 0;
            int end = possible_Trips.size();
            
            for (int  k = 1; k <= vehicle.getCapacity(); k++ ) {
                //Trip of size 1
                if(k == 1) {
                    for(int t =0; t< tempList.size();t++) {
                        vector<Request> list_Of_Temporary_Requests;
                        list_Of_Temporary_Requests.push_back(tempList[t]);
                        vector<float> pathDetails = pathGenerator.travel(vehicle, list_Of_Temporary_Requests);
                        //cout << "4 " << pathDetails[0] << endl;
                        if (pathDetails[0] != -1.0) {
                                Trip trip;
                                trip.addPassenger(tempList[t]);
                                trip.setTripCost(pathDetails[0]);
                                possible_Trips.push_back(trip);
                        }
                        list_Of_Temporary_Requests.clear();
                    }
                    end = possible_Trips.size();
                } //Trip size of 2 
                else if(k == 2){
                    for(int y = start; y < end; y++) {
                        for(int p = y+1; p < end; p++) {
                            vector<Request> list_Of_Temporary_Requests;
                            list_Of_Temporary_Requests.push_back(tempList[y]);
                            list_Of_Temporary_Requests.push_back(tempList[p]);
                            vector<float> pathDetails = pathGenerator.travel(vehicle, list_Of_Temporary_Requests);
                            //cout << "5 " << pathDetails[0] << endl;
                            if (pathDetails[0] != -1.0) {
                                Trip trip;
                                trip.addPassenger(tempList[y]);
                                trip.addPassenger(tempList[p]);
                                trip.setTripCost(pathDetails[0]);
                                possible_Trips.push_back(trip);
                            }
                            list_Of_Temporary_Requests.clear();
                        }
                    }
                    start = end;
                    end = possible_Trips.size();
                } //Trip size of 3 or more
                else {

                    for(int y = start; y < end; y++) {
                        for(int p = y + 1; p < end; p++) {
                            vector<Request> tempCombinedTrips;
                            Trip out_Trips = possible_Trips[y];
                            Trip in_Trips = possible_Trips[p];
                            tempCombinedTrips = out_Trips.getTripPassengers();
                            int sizet = tempCombinedTrips.size();

                            //create next level of trip size
                            for(int u = 0; u < in_Trips.getTripPassengers().size(); u++){
                                int checkVal = 1;
                                for(int r = 0; r < sizet; r++){
                                    //Trip is already present in the list or not
                                    if(tempCombinedTrips[r].getRequestID() == in_Trips.getTripPassengers()[u].getRequestID()) {
                                        checkVal = 0;
                                        break;
                                    }
                                }
                                if (checkVal) {
                                     tempCombinedTrips.push_back(in_Trips.getTripPassengers()[u]);
                                }
                            }
                            //only trips with considered capacity is allowed
                            if (tempCombinedTrips.size() == k){
                                vector<Request> temp;
                                for (int a = 0; a < k; a++) {
                                    for (int b = 0; b < k; b++) {
                                        if (b != a) {
                                            temp.push_back(tempCombinedTrips[b]);
                                        }
                                    }  
                                }
                                int topCheckCount = 0;
                                for(int l = 0; l < temp.size(); l = l +(k-1)) { 
                                   int mainCheckCount = 0;
                                   for(int j = start; j < end; j++) { 
                                       vector<Request> currentTripRequest = possible_Trips[j].getTripPassengers();
                                       int checkCount = 0;
                                       for (int w = 0; w < currentTripRequest.size(); w++) {
                                            for (int e = l; e < l + k-1 ; e++) {
                                                if(temp[e].getRequestID() == currentTripRequest[w].getRequestID()){
                                                    checkCount++;
                                                }
                                            }
                                        }
                                        if(checkCount == k-1) {
                                            mainCheckCount = 1;
                                            break;
                                        }
                                    }
                                    if (!mainCheckCount) {
                                        topCheckCount = 1;
                                        break;
                                    }
                               }
                               if(!topCheckCount) {
                                    vector<Request> list_Of_Temporary_Requests;
                                    for (int d = 0; d < tempCombinedTrips.size(); d++) {
                                        list_Of_Temporary_Requests.push_back(tempCombinedTrips[d]);
                                    }
                                    //cehck for duplicates 
                                    int duplicateCheck = 0;
                                    for (int z = end; z < possible_Trips.size(); z++) {
                                        vector<Request> currentRequests = possible_Trips[z].getTripPassengers();
                                        int trunCheck = 0;
                                        for (int d = 0; d < list_Of_Temporary_Requests.size(); d++) {
                                            int locCheck = 0;
                                            for (int x = 0; x < currentRequests.size(); x++) {
                                                if(list_Of_Temporary_Requests[d].getRequestID() == currentRequests[x].getRequestID()){
                                                    locCheck = 1;
                                                    trunCheck++;
                                                    break;
                                                } 
                                            }
                                            if(!locCheck) {
                                                break;
                                            }
                                        }
                                        if(trunCheck == k) {
                                            duplicateCheck = 1;
                                        }
                                    }
                                    if (!duplicateCheck) {
                                        vector<float> pathDetails = pathGenerator.travel(vehicle, list_Of_Temporary_Requests);
                                        //cout << "6 " << pathDetails[0] << endl;
                                        if (pathDetails[0] != -1.0) {
                                            Trip trip;
                                            for (int d =0; d < tempCombinedTrips.size(); d++) {
                                                trip.addPassenger(tempCombinedTrips[d]);
                                            }
                                            trip.setTripCost(pathDetails[0]);
                                            possible_Trips.push_back(trip);
                                        }
                                        list_Of_Temporary_Requests.clear();
                                    }
                               }
                            }
                        }
                    }
                    start = end;
                    end = possible_Trips.size();
                }
            }
            mtx.lock();
            list_Of_Trips[vehicle.getId()] = possible_Trips;
            cout << "[RIDESHARING] \033[0;35m 2. Possible Trip Size for Ride " << vehicle.getId() << "  is " << possible_Trips.size() << "" << endl;
            possible_Trips.clear();
            mtx.unlock();
        }      
    }
    pthread_exit(NULL);
}


//Function to find V-R edges of RV grapgh
void *Find_VREdges_Of_RVGraph(void *thread_obj)
{
    struct t_Data *thread_object;
    thread_object = (struct t_Data *) thread_obj;

    long thr_ID = (long)thread_object->thr_ID;
    long t_Count = (long)thread_object->no_Of_Thr;
    vector<Ride> list_Of_Taxis = (vector<Ride>)thread_object->list_Of_Taxis;
    vector<Request> list_Of_Requests = (vector<Request>)thread_object->list_Of_Requests;

    int no_Of_Taxis = list_Of_Taxis.size();
    OptimalPathGenerator pathGenerator;

    float countCheck = (no_Of_Taxis - (thr_ID)*(no_Of_Taxis/t_Count))/float(no_Of_Taxis/t_Count);

    // check for traversing all elements of the vector
    if(countCheck >= 1.0) {
        for(int i = ceil((thr_ID)*(no_Of_Taxis/(float)t_Count));  i < ceil((thr_ID+1)*(no_Of_Taxis/(float)t_Count)); i++) {
            vector<Request> tempRequestListForRideV;
            for (int j = 0; j < list_Of_Requests.size(); j++){
                vector<Request> list_Of_Temporary_Requests;
                list_Of_Temporary_Requests.push_back(list_Of_Requests[j]);
                vector<float> pathDetails = pathGenerator.travel(list_Of_Taxis[i], list_Of_Temporary_Requests);
                if (pathDetails[0] != -1.0) {
                    tempRequestListForRideV.push_back(list_Of_Requests[j]);
                    //cout << "[AMIT] request " << j << " request id is " << list_Of_Requests[j].getRequestID() << endl;
                }
                list_Of_Temporary_Requests.clear();
            }
            mtx.lock();
            list_Of_VR_Edges.insert(std::pair<int, vector<Request>> (list_Of_Taxis[i].getId(), tempRequestListForRideV));
            cout << "[RIDESHARING] \033[0;38m vehicle with id " << list_Of_Taxis[i].getId() << " has request list of size  " << tempRequestListForRideV.size(); //Seth
            for(vector<Request>::iterator j = tempRequestListForRideV.begin(); j < tempRequestListForRideV.end(); j++) {
                cout << " " << j->getRequestID();
            }
            cout << endl;
            tempRequestListForRideV.clear();
            mtx.unlock();
        }
    } else {
        for(int i = ceil((thr_ID)*(no_Of_Taxis/(float)t_Count));  i < no_Of_Taxis; i++) {
            vector<Request> tempRequestListForRideV;
            for (int j = 0; j < list_Of_Requests.size(); j++){
                vector<Request> list_Of_Temporary_Requests;
                list_Of_Temporary_Requests.push_back(list_Of_Requests[j]);
                vector<float> pathDetails = pathGenerator.travel(list_Of_Taxis[i], list_Of_Temporary_Requests);
                if (pathDetails[0] != -1.0) {
                    tempRequestListForRideV.push_back(list_Of_Requests[j]);
                    //cout << "[AMIT1] request " << j << " request id is " << list_Of_Requests[j].getRequestID() << endl;
                }
                list_Of_Temporary_Requests.clear();
            }  
            mtx.lock();
            list_Of_VR_Edges.insert(std::pair<int, vector<Request>> (list_Of_Taxis[i].getId(), tempRequestListForRideV));
            cout << "[RIDESHARING] \033[0;38m vehicle with id " << list_Of_Taxis[i].getId() << " has request list of size  " << tempRequestListForRideV.size(); //Seth
            for(vector<Request>::iterator j = tempRequestListForRideV.begin(); j < tempRequestListForRideV.end(); j++) {
                cout << " " << j->getRequestID();
            }
            cout << endl;
            tempRequestListForRideV.clear();
            mtx.unlock();
        }
    }
    cout << "[RIDESHARING] \033[0;38m List Of VR Edges has size        "  << list_Of_VR_Edges.size() << endl;
    pthread_exit(NULL);
}/////////////// checked : correct

//Function to find R-R edges of RV grapgh
void *Find_RREdges_Of_RVGraph(void *thread_obj)
{
    struct t_Data *thread_object;
    thread_object = (struct t_Data *) thread_obj;

    long thr_ID = (long)thread_object->thr_ID;
    long t_Count = (long)thread_object->no_Of_Thr;
    vector<Request> list_Of_Requests = (vector<Request>)thread_object->list_Of_Requests;

    int numberOfRequests = list_Of_Requests.size();
    OptimalPathGenerator pathGenerator;
    float count_Check = (numberOfRequests - (thr_ID)*(numberOfRequests/t_Count))/float(numberOfRequests/t_Count);
    /* check for traversing all elements of the vector */
    if(count_Check > 1.0) {
        for(int i = ceil((thr_ID)*(numberOfRequests/(float)t_Count));  i < ceil((thr_ID+1)*(numberOfRequests/(float)t_Count)); i++) {
            vector<Request> tempRequestListForRequestR;
            for (int j = 0; j < list_Of_Requests.size(); j++){
                vector<Request> list_Of_Temporary_Requests;
                list_Of_Temporary_Requests.push_back(list_Of_Requests[i]);
                list_Of_Temporary_Requests.push_back(list_Of_Requests[j]);

                Ride* dummyRide = new Ride(list_Of_Requests[i].getTripOriginLongitude(), 
                list_Of_Requests[i].getTripOriginLatitude(), 0.0, 0, "dummy", 4);

                vector<float> pathDetails = pathGenerator.travel(*dummyRide, list_Of_Temporary_Requests);
                if (pathDetails[0] != -1.0) {
                    tempRequestListForRequestR.push_back(list_Of_Requests[j]);
                }
                list_Of_Temporary_Requests.clear();
            }  
            mtx.lock();
            list_Of_RR_Edges[list_Of_Requests[i].getRequestID()] = tempRequestListForRequestR;
            tempRequestListForRequestR.clear();
            mtx.unlock();
        }
    } else {
        for(int i = ceil((thr_ID)*(numberOfRequests/(float)t_Count));  i < numberOfRequests; i++) {
            vector<Request> tempRequestListForRequestR;
            for (int j = 0; j < list_Of_Requests.size(); j++){
                vector<Request> list_Of_Temporary_Requests;
                list_Of_Temporary_Requests.push_back(list_Of_Requests[j]);

                Ride* dummyRide = new Ride(list_Of_Requests[i].getTripOriginLongitude(), 
                list_Of_Requests[i].getTripOriginLatitude(), 0.0, 0, "dummy", 4);

                vector<float> pathDetails = pathGenerator.travel(*dummyRide, list_Of_Temporary_Requests);
                if (pathDetails[0] != -1.0) {
                    tempRequestListForRequestR.push_back(list_Of_Requests[j]);
                }
                list_Of_Temporary_Requests.clear();
            }  
            mtx.lock();
            list_Of_RR_Edges[list_Of_Requests[i].getRequestID()] = tempRequestListForRequestR;
            tempRequestListForRequestR.clear();
            mtx.unlock();
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    cout  << "[RIDESHARING] \033[0;37m Starting the Ride Sharing" << endl;
    //number of threads
    int thr_Size = 1; //atoi(argv[1]);

    GeoHasher hasher;
    cout  << "[RIDESHARING] \033[0;37m Geo-hasing is loading" << endl;
    //setup the geo hash process
    hasher.initHasher();
    cout  << "[RIDESHARING] \033[0;37m Geo-hasing was loaded" << endl;
    
    DataReader reader;
    pthread_t threads[thr_Size];
    pthread_t threads1[thr_Size];
    pthread_t threads2[thr_Size];
    int rc;
    pthread_attr_t thread_attrib;
    pthread_attr_t thread_attrib1;
    pthread_attr_t thread_attrib2;
    void *status;
    time_t start,end,lap1, lap2;

    float serviceRate = 0;
    float adjustedServiceRate = 0;
    float occupancy = 0;
    float currentAvgWaiting = 0;
    float avgTotalDelay = 0;
    int totalAccumulatedRequestCount = 0;
    float avgComputationTime = 0;
    int iterationCount = 0;
    float maxComputationTime = 0;
    int vehicleSize = 0;
    int requestSize = 0;
    int serviceCounter = 0;
    int passengerDropCount = 0;
    int previousRideCount = 0;

    /* Get the vehicle list */
    cout << "[RIDESHARING] \033[0;35m Get the Ride List" << endl;
    vector<Ride> list_Of_Taxis = reader.obtainTaxiData();
    cout << "[RIDESHARING] \033[0;35m Ride List size is " << list_Of_Taxis.size() << "" << endl;

    /* Get the request list */
    cout << "[RIDESHARING] \033[0;35m Get the Request List" << endl;
    vector<Request> list_Of_Requests = reader.obtainHailingData();
    cout << "[RIDESHARING] \033[0;35m Request List size is " << list_Of_Requests.size() << "" << endl;

    BufferManager bufferManager;
    int initTime = 070000;
    vector<Request> HailingList = list_Of_Requests;
    cout << "[RIDESHARING] \033[0;36m Iterations Started" << endl;

    /* Iterates every Delta seconds */
    while (true) {

        /* Initial time incremented by 30 seconds for every iteration */ 
        initTime = bufferManager.updateTimeStamp(initTime);
        cout << "[RIDESHARING] \033[0;35m Updated the buffer time stamp to :" << initTime <<"" << endl;
        cout << "[RIDESHARING] \033[0;33m Updating the current request buffer" << endl;

        vector<Request> notYetServiced;
        for(int x = 0; x < HailingList.size(); x++)  {
            Request currentRequest = HailingList[x];

            int checkA = 1;
            for(int y = 0; y < list_Of_Taxis.size(); y++) {
                Ride vehicle = list_Of_Taxis[y];
                vector<Request> passengers = vehicle.getPassengerList();
                int checkP = 1;
                for(int n = 0; n < passengers.size(); n++) {
                    if(passengers[n].getRequestID() == currentRequest.getRequestID()) {
                        checkP = 0;
                        break;
                    }
                }
                if(!checkP) {
                    checkA = 0;
                    break;
                }
            }

            /* if the passenger is not yet serviced, increment waiting time by 30 sec */
            if(checkA)  {
                HailingList[x].addToWaitingTime(30.0);
                HailingList[x].addToTotalServicedTime(30.0);
                notYetServiced.push_back(HailingList[x]);
            }
        }

        HailingList.clear();
        HailingList = notYetServiced;

        for(int u = 0; u < HailingList.size(); u++) {
            float timeWait = HailingList[u].getWaitingTime();
            float oldAvgWaiting = currentAvgWaiting * serviceCounter;
            serviceCounter++;
            currentAvgWaiting = (oldAvgWaiting + timeWait) / serviceCounter;
        }

        cout << "[RIDESHARING] \033[0;32m Current request buffer is updated" << endl;

        /* remove the in-vehicle passengers who have exceeded limits */
        cout << "[RIDESHARING] \033[0;33m Updating in-vehicle Passengers" << endl;
        for(int y = 0;  y < list_Of_Taxis.size(); y++) {
            Ride vehicle = list_Of_Taxis[y];
            vector<Request> passengers = vehicle.getPassengerList();
            vector<Request> newRequests;
            for(int x = 0; x < passengers.size(); x++) {
                if(passengers[x].getTripDestinationLatitude() == vehicle.getLatitude() && passengers[x].getTripDestinationLongitude() == vehicle.getLongitude()) {

                        /* ignore this passenger for next allocation as he is serviced already */
                        cout << "[RIDESHARING] \033[0;35m " << "Latitude Longitude obtained" << endl;
                        float oldDelay = avgTotalDelay * passengerDropCount;
                        passengerDropCount++;
                        avgTotalDelay = (oldDelay + passengers[x].getTotalServicedTime() - passengers[x].getidealTravelDelay()) /passengerDropCount;
                } else {
                    Constants constants;
                    cout << "[RIDESHARING] \033[0;35m " << "Latitude Longitude not obtained" << endl;
                    if(passengers[x].getTotalServicedTime() < constants.getMaxDelay() + constants.getMaxWaitingTime()) {
                        newRequests.push_back(passengers[x]);
                    } else {
                        /* ignore this passenger as well for next allocation as he is delayed beyond quality of service guarantee */
                        float earlierDelay = avgTotalDelay * passengerDropCount;
                        cout << "[RIDESHARING] \033[0;35m " << "Maximun delay or waiting time both not satisfied" << endl;
                        passengerDropCount++;
                        avgTotalDelay = (earlierDelay + passengers[x].getTotalServicedTime() - passengers[x].getidealTravelDelay()) /passengerDropCount;
                    }
                }
            }
            vehicle.setPassengerList(newRequests);
            newRequests.clear();
        }
        cout  << "[RIDESHARING] \033[0;32m In-vehicle passenger update is done " << endl;

        time(&start);

        /* load the vehicle and request buffer */

        /* get the vehicle list for the last MAX_STAY_TIME (4) hours */
        cout  << "[RIDESHARING] \033[0;33m Running buffer update" << endl;
        vector<Ride> bufferRideList = bufferManager.getBufferTaxis(list_Of_Taxis, initTime);

        /* get the new request list in the last 30 sec  */
        vector<Request> bufferTempRequestList = bufferManager.getBufferRequests(list_Of_Requests, initTime);

        /* add the new requests to the existing vehicel buffer */
        for(int k = 0; k < bufferTempRequestList.size(); k++) {
            HailingList.push_back(bufferTempRequestList[k]);
        }

        vehicleSize = bufferRideList.size();
        requestSize = HailingList.size();

        cout << "[RIDESHARING] \033[0;35m Request Size is " << requestSize << "" << endl;
        cout << "[RIDESHARING] \033[0;35m Ride Size is " << vehicleSize << endl;

        /* update occupancy rate */
        for(int t = 0; t < bufferRideList.size(); t++) {
            int pSize = bufferRideList[t].getPassengerList().size();
            float earlierOccupancy = occupancy * previousRideCount;
            if(pSize > 0 ){ 
                previousRideCount++;
                occupancy = (earlierOccupancy + pSize)/(previousRideCount *1.0);
            }
        }

        time (&lap1);
        double dif1 = difftime (lap1,start);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif1) <<" Buffer update completed" << endl;
        
        /* Initialize and set joinable attribute thread  */
        pthread_attr_init(&thread_attrib);
        pthread_attr_setdetachstate(&thread_attrib, PTHREAD_CREATE_JOINABLE);

        /************* Build R-V edges of RV graph **********/
        cout  << "[RIDESHARING] \033[0;33m Building R-V edges of RV graph" << endl; /////////////////////SETH ///////////////////
        //struct t_Data thread_obj_array[thr_Size];
        struct t_Data *thread_obj_array = new t_Data[thr_Size];
        for(int t = 0; t < thr_Size; t++){
    
            //struct t_Data thread_obj;
            thread_obj_array[t].thr_ID = t;///////
            thread_obj_array[t].no_Of_Thr = thr_Size;
            thread_obj_array[t].list_Of_Taxis = bufferRideList;
            thread_obj_array[t].list_Of_Requests = HailingList;

            rc = pthread_create(&threads[t], &thread_attrib, Find_VREdges_Of_RVGraph, (void *)&thread_obj_array[t]);
            if (rc) {
                printf("[RIDESHARING] \033[0;31m ERROR; return code from pthread_create() is %d\033[0m \n", rc);
                exit(-1);
            }
        }

        // Free attribute and wait for the other threads
        pthread_attr_destroy(&thread_attrib);

        for(long t = 0; t < thr_Size; t++) {
                
            rc = pthread_join(threads[t], NULL);
            if (rc) {
                printf("[RIDESHARING] \033[0;31m ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }

     
        time (&lap2);
        double dif2 = difftime (lap2,lap1);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif2) <<" R-V edges of RV graph is completed" << endl;

        /************** Build R-R edges of RV graph ***************/
        cout  << "[RIDESHARING] \033[0;33m Building R-R edges of RV graph" << endl;  
        // Initialize and set thread detached attribute
        pthread_attr_init(&thread_attrib1);
        pthread_attr_setdetachstate(&thread_attrib1, PTHREAD_CREATE_JOINABLE);

        for(int t = 0; t < thr_Size; t++){

            //struct t_Data thread_obj;
            thread_obj_array[t].thr_ID = t;//////
            thread_obj_array[t].no_Of_Thr = thr_Size;
            thread_obj_array[t].list_Of_Requests = HailingList;

            rc = pthread_create(&threads1[t], &thread_attrib1, Find_RREdges_Of_RVGraph, (void *)&thread_obj_array[t]);
            if (rc) {
                printf("[RIDESHARING] \033[0;32m ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
    
        // Free attribute and wait for the other threads
        pthread_attr_destroy(&thread_attrib1);
        for(long t = 0; t < thr_Size; t++) {
            rc = pthread_join(threads1[t], NULL);
            if (rc) {
                printf("[RIDESHARING] \033[0;32m ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        time (&lap1);
        double dif3 = difftime (lap1,lap2);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif3) <<" R-R edges of RV graph is completed" << endl;

        /************** Build RTV graph ***************/
        cout  << "[RIDESHARING] \033[0;33m Building RTV graph" << endl;
        // Initialize and set thread detached attribute
        pthread_attr_init(&thread_attrib2);
        pthread_attr_setdetachstate(&thread_attrib2, PTHREAD_CREATE_JOINABLE);
        struct t_Data_RTV thread_obj_array_rtv[thr_Size];
        //struct t_Data_RTV *thread_obj_array_rtv = new t_Data_RTV[thr_Size];
        for(int t = 0; t < thr_Size; t++){

            thread_obj_array_rtv[t].thr_ID = t;
            thread_obj_array_rtv[t].no_Of_Thr = thr_Size;
            thread_obj_array_rtv[t].list_Of_RR_Edges = list_Of_RR_Edges;
            thread_obj_array_rtv[t].list_Of_VR_Edges = list_Of_VR_Edges;
            thread_obj_array_rtv[t].list_Of_Taxis = list_Of_Taxis;

            rc = pthread_create(&threads2[t], &thread_attrib2, Make_RTV_Graph, (void *)&thread_obj_array_rtv[t]);
            if (rc) {
                printf("[RIDESHARING] \033[0;32m ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
    
        // Free attribute and wait for the other threads
        pthread_attr_destroy(&thread_attrib2);
        for(long t = 0; t < thr_Size; t++) {
            rc = pthread_join(threads2[t], NULL);
            if (rc) {
                printf("[RIDESHARING] \033[0;32m ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        time (&lap2);
        double dif4 = difftime (lap2,lap1);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif4) <<"  RTV graph is completed" << endl;

        cout << "[RIDESHARING] \033[0;33m Solve ILP problem" << endl;
        //Solve assignment as a ILP 
        MosekSolution ilpSolver;
        cout << "[RIDESHARING] \033[0;33m Taxis " << bufferRideList.size() << " - Requests "  << HailingList.size() << endl;
        for (auto& j: list_Of_Trips) {
            int vehicleID = j.first;
            vector<Trip> trips = j.second;
            cout << "[RIDESHARING] \033[0;33m Ride Id " << vehicleID << " can handle " << trips.size() << " trips : ";
            for(vector<Trip>::iterator k = trips.begin(); k < trips.end(); k++) {
                cout << " " << k->getTripDetails();
            }
            cout << endl;
        }
        vector<int> finalAssignment = ilpSolver.solve(list_Of_Trips, HailingList);
        time (&lap1);
        double dif5 = difftime (lap1, lap2);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif5) <<"  ILP problem solving completed" << endl;
        cout  << "[RIDESHARING] \033[0;33m Ride re-balancing started" << endl;
        vector<Ride> unasignedRides;
        //compute unassigned vehicles [stopped, not allocated]
        for (int y = 0; y < bufferRideList.size(); y++) {
            if (bufferRideList[y].getPassengerList().size() > 0) {
                continue;
            }
            int check = 1;
            int size = 0;
            for (auto& j: list_Of_Trips) {
                int vehicleID = j.first;
                vector<Trip> trips = j.second;
                if(vehicleID == bufferRideList[y].getId()) {
                    for (int k = (size); k < (size) + trips.size(); k++) {
                        if(finalAssignment[k] == 1) {
                            check = 0;
                        }
                    }
                }
                size = size + trips.size();
            }
            if(check) {
                    unasignedRides.push_back(bufferRideList[y]);
            }
        } 

        vector<Request> unasignedRequest;
        //compute unassigned requests
        for (int y = 0; y < HailingList.size(); y++) {
            int size = 0;
            int check = 1;
            int breakCheck = 0;
            for (auto& j: list_Of_Trips) {
                vector<Trip> trips = j.second;
                for (int k = 0; k < trips.size(); k++) {
                    vector<Request> requests = trips[k].getTripPassengers();
                    for (int u = 0; u < requests.size(); u++) {
                        if(requests[u].getRequestID() == HailingList[y].getRequestID()){
                            if(finalAssignment[size+k]) {
                                check = 0;
                                breakCheck = 1;
                                break;
                            }
                        }
                    }
                    if(breakCheck){
                        break;
                    }
                }
                if(breakCheck){
                        break;
                }
                size = size + trips.size();
            }
            if(check) {
                    unasignedRequest.push_back(HailingList[y]);
            }
        }

        OptimalPathGenerator pathGenerator;
        vector<int> finalReBalanceAssignment;
        map<int, vector<Trip> > unassignedTripList;
        cout << "[RIDESHARING] \033[0;35m Unassigned Taxis " << unasignedRides.size() << " Unhandled Requests " << unasignedRequest.size() << "" << endl;
        if (unasignedRides.size() != 0 && unasignedRequest.size() != 0) {
            unassignedTripList = pathGenerator.getTravelCosts(unasignedRides, unasignedRequest);
            //solve re-balancing as an ILP
            finalReBalanceAssignment = ilpSolver.solveForRebalancing(unassignedTripList , unasignedRequest, unasignedRides);
        }

        int reBalanceCount = 0;
        for (int i = 0; i < finalReBalanceAssignment.size(); i = i + unasignedRequest.size()) {
            if(finalReBalanceAssignment[i] == 1) {
                reBalanceCount ++;
            }
        }
       
        time (&lap2);
        double dif6 = difftime (lap2, lap1);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif6) <<"  Ride re-balancing completed" << endl;

         /* take the time it took to calculate */
        time (&end);
        double dif = difftime (end,start);

        cout  << "[RIDESHARING] \033[0;33m Ride simulation started" << endl;
        map<int, Trip> finalTripList;

        //get the final vehicle - trip combination from assignment
        int count = 0;
        for (auto& j: list_Of_Trips) {
            int vehicleID = j.first;
            vector<Trip> trips = j.second;
            for(int r = 0; r < trips.size(); r++) {
                if(finalAssignment[r + count] == 1) {
                    finalTripList[vehicleID] = trips[r];
                    break;
                }
            }
            count = count + trips.size();
        }

        /* get the final vehicle - trip combination from re-balancing */
        count = 0;
        for (auto& j: unassignedTripList) {
            int vehicleID = j.first;
            vector<Trip> trips = j.second;
            for(int r = 0; r < trips.size(); r++) {
                if(finalReBalanceAssignment[r + count] == 1) {
                    finalTripList[vehicleID] = trips[r];
                    break;
                }
            }
            count = count + trips.size();
        }
        
        /* get final allocated request list */
        vector<Request> finalAllocatedRequestList;
        for (auto& j: finalTripList) {
            Trip trip = j.second;
            for(int e = 0 ; e < trip.getTripPassengers().size() ; e++) {
                finalAllocatedRequestList.push_back(trip.getTripPassengers()[e]);
            }
        }
        cout << "[RIDESHARING] \033[0;35m Final Trip List " << finalTripList.size() << "" << endl;
        for (auto& j:finalTripList) {
            Trip trip = j.second;
            cout << " [" << j.first << " --> {" << trip.getTripDetails() << "} ], ";
        }
        cout << "\033[0m \n " << endl;

        Simulator simulator;
        /* simulate the vehicles */
        vector<Ride> tempV = list_Of_Taxis;
        list_Of_Taxis = simulator.simulateRide(tempV, finalTripList);

        time (&lap1);
        double dif7 = difftime (lap1, lap2);
        cout  << "[RIDESHARING] \033[0;32m " << to_string(dif7) <<"  Ride simulation completed" << endl;

        /* calculate statistic summary details */
        int serviceCount =  HailingList.size() - unasignedRequest.size();
        int requestCount = HailingList.size();
        serviceRate = ((serviceRate/100 * totalAccumulatedRequestCount + serviceCount) / (totalAccumulatedRequestCount + requestCount) ) * 100;

        adjustedServiceRate = ((adjustedServiceRate/100 * totalAccumulatedRequestCount + serviceCount + reBalanceCount) / (totalAccumulatedRequestCount + requestCount) ) * 100;
        totalAccumulatedRequestCount = totalAccumulatedRequestCount + requestCount;

        iterationCount++;
        avgComputationTime = (avgComputationTime * (iterationCount -1) + dif) / iterationCount;

        if(maxComputationTime < dif) {
            maxComputationTime = dif;
        }

        cout  << "\033[0;37m[SUMMARY] Iteration Computation Time = " << to_string(dif) << " s" << endl;
        cout  << "\033[0;37m[SUMMARY] Number of vehicles available = " << to_string((int)vehicleSize) << " " << endl;
        cout  << "\033[0;37m[SUMMARY] Number of pending requests = " << to_string((int)requestSize) << " " << endl;
        cout  << "\033[0;37m[SUMMARY] Average Computation Time = " << to_string(avgComputationTime) << " s" << endl;
        cout  << "\033[0;37m[SUMMARY] Maximum Computation Time = " << to_string(maxComputationTime) << " s" << endl;
        cout  << "\033[0;37m[SUMMARY] Service Rate = " << to_string((int)serviceRate) << " %" << endl;
        cout  << "\033[0;37m[SUMMARY] Service Rate After Re-Balancing = " << to_string((int)adjustedServiceRate) << " %" << endl;
        cout  << "\033[0;37m[SUMMARY] Average Waiting Time = " << to_string(currentAvgWaiting) << " s" << endl;
        cout  << "\033[0;37m[SUMMARY] Average Total Travel Delay = " << to_string(avgTotalDelay) << " s" << endl;
        cout  << "\033[0;37m[SUMMARY] Occupancy (Passengers/Ride) = " << to_string(occupancy) << " " << endl;
        cout << "\033[0m \n " << endl;

        /* reset the data structures for next iteration */
        list_Of_VR_Edges.clear();
        list_Of_RR_Edges.clear();
        list_Of_Trips.clear();
        bufferRideList.clear();
        HailingList.clear();

        HailingList = finalAllocatedRequestList;
        // sleep a total of 30 seconds
        //sleep(INTERVAL - dif);
        //cout<< list_Of_VR_Edges.size() <<endl;
        cout  << "[RIDESHARING] \033[0;32m Done with iteration " << endl;

    }
    pthread_exit(NULL);
}
