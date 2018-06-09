#include <stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "../headers/OSRMCaller.h"

using namespace std;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//function to create the duration matrix for given location points
vector<vector<float> > OSRMCaller::getMatrix(string url){

    const char *cstr = url.c_str();

    CURL *curl;
    CURLcode res;
    string readBuffer;
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, cstr);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
    }

    vector<vector<float> > matrix;

    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( readBuffer, root );

    auto durations = root["durations"];
        
    for ( int i = 0; i < durations.size(); i++ )
    {
        vector<float> row;
        for ( int k = 0; k< durations.size(); k++ )
        {
            float val = durations[i][k].asFloat();
            row.push_back(val);
        }
        matrix.push_back(row);
        row.clear();
    }

    return matrix;
}