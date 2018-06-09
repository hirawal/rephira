#include "../headers/GeoHasher.h"
#include <string.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath> 
#include <string>
using namespace std;

#define GEO_HASH_PRECISION 8
#define EARTH_RADIUS_KM 6371.0

extern "C"
{
    #include "geohash.h"
    char* geohash_encode(double, double, int );
    char** geohash_neighbors(char*);
    GeoCoord geohash_decode(char*);
}

// Order - ex : 79.82323, 6.0232  
vector<vector <float> > nodeLocationList; 
vector<string> nodeHashValueList; 


void GeoHasher::initHasher(){
    // setup node details
    getNodeData();
    // setup hash values 
    getHashData();

}

//load the node data - longitude and latitude
void GeoHasher::getNodeData() {  
  ifstream ip("data/nodeList.csv");

  if(!ip.is_open()) std::cout << "ERROR: File Open in nodeList.csv" << endl;

  string longitude;
  string latitude;
  string nodeID;

  while(ip.good()){

    getline(ip,nodeID,',');
    getline(ip,latitude,',');
    getline(ip,longitude,'\n');

    vector<float> tempList;
    tempList.push_back(stof(latitude));
    tempList.push_back(stof(longitude));

    nodeLocationList.push_back(tempList);

    tempList.clear();
  }
  ip.close();
}

//load the hash values corresponding to node data 
void GeoHasher::getHashData() {  
  ifstream ip("data/hashList.csv");

  if(!ip.is_open()) std::cout << "ERROR: File Open in hashList.csv" << endl;

  string hashValue;
  string nodeID;

  while(ip.good()){

    getline(ip,nodeID,',');
    getline(ip,hashValue,'\n');

    nodeHashValueList.push_back(hashValue);
  }
  ip.close();
}

vector<float> GeoHasher::getNearestNode(double latitude, double longitude) {
    vector<char*> neighbourHasValues;
   
    //hash the given localtion value
    char* hash = geohash_encode(latitude, longitude, GEO_HASH_PRECISION);
    neighbourHasValues.push_back(hash);

    vector<int> tempMatches;
    int count = 0;
    int testCount = 0;
    while(tempMatches.size() < 1) {
        // do not wait more than 1 iterations
        if(testCount > 1) {
            vector<float> returnValueList;
            returnValueList.push_back(0.00);
            returnValueList.push_back(0.0);
            returnValueList.push_back(1000.0);
            returnValueList.push_back(0);

        return returnValueList;
        }
        testCount++;
        int initIterationCount = neighbourHasValues.size();
        for (int k = count; k < initIterationCount; k++) {
            //get the 8 neighbours surrounding the given location hash bucket and the bukcet itself
            char** neighbors = geohash_neighbors(neighbourHasValues[k]);

            int iterations  = neighbourHasValues.size();
            for(int i = 0 ; i < 8 ; i ++ ){
                //check for duplicates
                bool check = true;
                for(int p = 0 ; p < iterations ; p++ ){
                    string temp_1 = neighbourHasValues[p];
                    string temp_2 = neighbors[i];
                    if(temp_1.compare(temp_2) == 0){
                        check = false;
                    }
                }
                if (check) {
                    neighbourHasValues.push_back(neighbors[i]);
                }
            }
        }

        //update count value
        if (count == 0) {
            count = count + 1;
        } else {
            count = count + 2;
        }

        //get the matching hash backets 
        for(int i = 0 ; i < nodeHashValueList.size() ; i ++ ){
            if (nodeHashValueList[i] == hash) { 
                    tempMatches.push_back(i);
                } 
            for(int j = count*count ; j < (count + 2) * (count + 2) ; j ++ ) {
                if (nodeHashValueList[i] == neighbourHasValues[j]) {
                    tempMatches.push_back(i);
                } 
            }
        }
    }

    double minLatitude = 0;
    double minLongitude = 0;
    double minDistance = 10000.0;
    int minIndex = 0;

    for (int y = 0; y < tempMatches.size(); y++) {
        double distance = distanceEarth(latitude, longitude, 
                                nodeLocationList[tempMatches[y]][1], nodeLocationList[tempMatches[y]][0]);
        if(minDistance > distance) {
            minDistance = distance;
            minLatitude = nodeLocationList[tempMatches[y]][1];
            minLongitude = nodeLocationList[tempMatches[y]][0];
            minIndex = tempMatches[y];
        }
    }
    vector<float> returnValueList;
    returnValueList.push_back(minLatitude);
    returnValueList.push_back(minLongitude);
    returnValueList.push_back(minDistance*60);
    returnValueList.push_back(minIndex);

    return returnValueList;
}

// This function converts decimal degrees to radians
double GeoHasher::deg2rad(double deg) {
  return (deg * M_PI / 180);
}

//  This function converts radians to decimal degrees
double GeoHasher::rad2deg(double rad) {
  return (rad * 180 / M_PI);
}

/**
 * Returns the distance between two points on the Earth.
 * Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The distance between the two points in kilometers
 */
double GeoHasher::distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  u = sin((lat2r - lat1r)/2);
  v = sin((lon2r - lon1r)/2);
  return 2.0 * EARTH_RADIUS_KM * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}


vector<float> GeoHasher::getNearestNodes(double latitude, double longitude) {
    vector<char*> neighbourHasValues;
   
    //hash the given localtion value
    char* hash = geohash_encode(latitude, longitude, GEO_HASH_PRECISION);
    neighbourHasValues.push_back(hash);

    vector<int> tempMatches;
    int count = 0;
    int testCount = 0;
    while(tempMatches.size() < 3) {
        // do not wait more than 1 iterations
        if(testCount > 2) {
            vector<float> returnValueList;
            return returnValueList;
        }
        testCount++;
        int initIterationCount = neighbourHasValues.size();
        for (int k = count; k < initIterationCount; k++) {
            //get the 8 neighbours surrounding the given location hash bucket and the bukcet itself
            char** neighbors = geohash_neighbors(neighbourHasValues[k]);

            int iterations  = neighbourHasValues.size();
            for(int i = 0 ; i < 8 ; i ++ ){
                //check for duplicates
                bool check = true;
                for(int p = 0 ; p < iterations ; p++ ){
                    string temp_1 = neighbourHasValues[p];
                    string temp_2 = neighbors[i];
                    if(temp_1.compare(temp_2) == 0){
                        check = false;
                    }
                }
                if (check) {
                    neighbourHasValues.push_back(neighbors[i]);
                }
            }
        }

        //update count value
        if (count == 0) {
            count = count + 1;
        } else {
            count = count + 2;
        }

        //get the matching hash backets 
        for(int i = 0 ; i < nodeHashValueList.size() ; i ++ ){
            if (nodeHashValueList[i] == hash) { 
                    tempMatches.push_back(i);
                } 
            for(int j = count*count ; j < (count + 2) * (count + 2) ; j ++ ) {
                if (nodeHashValueList[i] == neighbourHasValues[j]) {
                    tempMatches.push_back(i);
                } 
            }
        }
    }
    vector<float> returnValueList;
    for (int y = 0; y < tempMatches.size(); y++) {
        returnValueList.push_back(nodeLocationList[tempMatches[y]][1]);
        returnValueList.push_back(nodeLocationList[tempMatches[y]][0]);
    }

    return returnValueList;
}