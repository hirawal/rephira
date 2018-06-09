#include "../headers/DataReader.h"
#include "../headers/Ride.h"
#include "../headers/Request.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

vector<Ride> DataReader::obtainTaxiData() {
    
  std::vector<Ride> list_Of_Taxis; 
  
  ifstream ip("data/taxiData.csv");

  if(!ip.is_open()) std::cout << "ERROR: File Open1" << endl;

  string driverId;
  string longitude;
  string latitude;
  string speed = "30.0";
  string time;
  string capacity;
  getline(ip, driverId,'\r');
  while(ip.good()){
    getline(ip, driverId,',');
    getline(ip,latitude,',');
    getline(ip,longitude,',');
    getline(ip,time,',');
    getline(ip,capacity,'\n');
    //cout << "[RIDESHARING] \033[0;30m " << atoi(driverId.c_str()) << ", " << latitude << ", " << longitude << ", " << time << ", " << capacity << endl; 
    if (atoi(driverId.c_str()) == 0) driverId = "1";
    Ride vehicle(strtof((longitude).c_str(),0), strtof((latitude).c_str(),0),  strtof((speed).c_str(),0), atoi(driverId.c_str()), time, atoi(capacity.c_str()));
    list_Of_Taxis.push_back(vehicle);
  }
  ip.close();

  return list_Of_Taxis;
}

vector<Request> DataReader::obtainHailingData(){
    
  std::vector<Request> list_Of_Requests;
    
  ifstream ip("data/requestData.csv");
  
  if(!ip.is_open()) std::cout << "ERROR: File Open" << endl;

  string requestID;
  string tripOriginLongitude;
  string tripOriginLatitude;
  string tripDestinationLongitude;
  string tripDestinationLatitude;
  string requestedTime;

  while(ip.good()){

    getline(ip, requestID,',');
    getline(ip,tripOriginLatitude,',');
    getline(ip,tripOriginLongitude,',');
    getline(ip,tripDestinationLatitude,',');
    getline(ip,tripDestinationLongitude,',');
    getline(ip,requestedTime,'\n');
    if (atoi(requestID.c_str()) == 0) requestID = "1";
    //cout << "[RIDESHARING] \033[0;30m " << requestID << ", " << tripOriginLatitude << ", " << tripOriginLongitude << ", " << tripDestinationLatitude << ", " << tripDestinationLongitude << ", " << requestedTime << endl; 
    Request request(strtof((tripOriginLongitude).c_str(),0), 
            strtof((tripOriginLatitude).c_str(),0), 
            strtof((tripDestinationLongitude).c_str(),0), 
            strtof((tripDestinationLatitude).c_str(),0),
            atoi(requestID.c_str()), requestedTime);
    
    list_Of_Requests.push_back(request); 
  }

  ip.close();
  
  return list_Of_Requests;
}
