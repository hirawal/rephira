#include <vector>
#include "Ride.h"
#include "Request.h"

#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

using namespace std;

class BufferManager {

public:
    vector<Ride> getBufferTaxis(vector<Ride> list_Of_Taxis, int startTime);
    vector<Request> getBufferRequests(vector<Request> list_Of_Requests, int startTime);
    int updateTimeStamp(int currentTime);

private:
    int getPreviousTimeStamp(int currentTimeStamp);
};

#endif /* BUFFERMANAGER_H */

