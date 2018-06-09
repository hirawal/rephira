#include "../headers/BufferManager.h"
#include "../headers/Ride.h"
#include "../headers/Request.h"
#include <iostream>
#include <fstream>
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

#define MAX_STAY_TIME 4 //4 hours
using namespace std;

vector<Ride> BufferManager::getBufferTaxis(vector<Ride> list_Of_Taxis, int thisTime) {
    vector<Ride> vehicleBuffer;
    int vehicleListSize = list_Of_Taxis.size();
    cout << "[RIDESHARING] \033[0;35m getBufferTaxis::vehicleListSize is " << vehicleListSize << endl;
    for(int i = 0; i < vehicleListSize; i++) {
        string stringValue;
        string joinTime = list_Of_Taxis[i].getJoinedTime();
        cout << "[RIDESHARING] \033[0;32m Drver Id is " << list_Of_Taxis[i].getId() << endl;
        istringstream ss(joinTime);
        string token;
        while(getline(ss, token, ':')) {
            stringValue = stringValue + token;
        }
        int value = stoi(stringValue);

         /* add all the vehicle within the time gap of MAX_STAY_TIME hours */
        if(value <= thisTime && ((thisTime -value)/10000) < MAX_STAY_TIME) {
            vehicleBuffer.push_back(list_Of_Taxis[i]);
        }
    }

    return vehicleBuffer;
}

vector<Request> BufferManager::getBufferRequests(vector<Request> list_Of_Requests, int thisTime) {
    
    vector<Request> requestBuffer;
    int requestListSize = list_Of_Requests.size();

    for(int i = 0; i < requestListSize; i++) {
        string stringValue;
        string reqTime = list_Of_Requests[i].getRequestedTime();
        istringstream ss(reqTime);
        string token;
        while(getline(ss, token, ':')) {
            stringValue = stringValue + token;
        }
        int value = stoi(stringValue);
        if(value <= thisTime &&  value > getPreviousTimeStamp(thisTime)) {
            requestBuffer.push_back(list_Of_Requests[i]);
        }
    }

    return requestBuffer;
}


/* update the buffer time every 30 seconds */
int BufferManager::updateTimeStamp(int currentTime){
    string currentTimeValue;

    stringstream ss;
    ss << currentTime;
    currentTimeValue = ss.str();
    string tmp = "";
    if(currentTimeValue.size() != 6){
        for (int y = 0; y < 6 - currentTimeValue.size(); y++) {
            tmp = tmp + "0";
        } 
    }
    currentTimeValue  = tmp + currentTimeValue;

    string hour, minute, second;
    hour = (currentTimeValue.substr(0,2));
    minute = (currentTimeValue.substr(2,2));
    second = (currentTimeValue.substr(currentTimeValue.size() - 2, currentTimeValue.size()));

    if (second == "30") {
        if(minute == "59") {
            second = "00";
            minute = "00";
            int tempHour = (stoi(hour) + 1);
            stringstream hourSS;
            hourSS << tempHour;
            hour  = hourSS.str();
            if(hour.size() == 1) {
                hour = "0" + hour;
            }
        } else {
            int tempMinute = stoi(minute) + 1;
            stringstream minuteSS;
            minuteSS << tempMinute;
            minute  = minuteSS.str();
            if(minute.size() == 1) {
                minute  = "0" + minute; 
            }
            second = "00";
        }
    } else {
        second = "30";
    }

    string newTime = hour + minute + second;

    return stoi(newTime);
}

/* get the previous time stamp coresponding to 30 second time gap */
int BufferManager::getPreviousTimeStamp(int currentTimeStamp){
    stringstream endTimeSS;
    endTimeSS << currentTimeStamp;
    string endTimeValue  = endTimeSS.str();

    string tmp = "";
    if(endTimeValue .size() != 6){
        for (int y = 0; y < 6 - endTimeValue .size(); y++) {
            tmp = tmp + "0";
        } 
    }
    endTimeValue   = tmp + endTimeValue ;

    string second = endTimeValue.substr(endTimeValue.size()-2, endTimeValue.size());
    string minute = endTimeValue.substr(2, 2);
    string hour = endTimeValue.substr(0, 2);

    if(second == "00") {
        if(minute == "00") {
            second = "30";
            minute = "59";
            int tempHour = (stoi(hour) - 1);
            stringstream hourSS;
            hourSS << tempHour;
            hour  = hourSS.str();
            if(hour.size() == 1) {
                hour = "0" + hour;
            }
        } else {
            int tempMinute = stoi(minute) - 1;
            stringstream minuteSS;
            minuteSS << tempMinute;
            minute  = minuteSS.str();
            if(minute.size() == 1) {
                minute  = "0" + minute; 
            }
            second = "30";

        }
    } else {
        second = "00";
    }

    string newTime = hour + minute + second;

    return stoi(newTime);
}
