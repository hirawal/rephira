#include "../headers/MosekSolution.h"
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
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include "fusion.h"
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <boost/any.hpp>
#include <typeinfo>

using namespace mosek::fusion;
using namespace monty;
using namespace std;

#define MISS_COST 1000.0;

typedef std::shared_ptr<monty::ndarray<double,1> > IntPtr;

//the function to solve the ILP
vector<int> MosekSolution::solve(map<int, vector<Trip> > list_Of_Trips, vector<Request> list_Of_Requests) {
    //remove duplicates and create final trip list
    /*
    vector<Trip> finalTripSet;
    vector<vector<int> > indexList;
     for (auto& x: list_Of_Trips) {
        vector<Trip> trips = x.second ;
        vector<int> tempIndexList;
        for (int y = 0; y < trips.size(); y++) {
            int checkCount = 0;
            for (int r = 0; r < finalTripSet.size(); r++) {
                if(finalTripSet[r].getTripPassengers().size() == trips[y].getTripPassengers().size()){
                    vector<Request> outerPassengerList = trips[y].getTripPassengers();
                    vector<Request> innerPassengerList = finalTripSet[r].getTripPassengers();
                    int localCheck = 0;
                    for(int t = 0; t < outerPassengerList.size(); t++ ){
                        for(int b = 0; b < innerPassengerList.size(); b++ ){
                            if(outerPassengerList[t].getRequestID() == innerPassengerList[t].getRequestID()){
                                localCheck++;
                                break;
                            }
                        }
                    }
                    if(localCheck == innerPassengerList.size()) {
                        tempIndexList.push_back(r);
                        checkCount = 1;
                        break;
                    }  
                }
            }
            if(!checkCount) {
                tempIndexList.push_back(finalTripSet.size());
                finalTripSet.push_back(trips[y]);
            }
        }
        indexList.push_back(tempIndexList);
    }
    */
    vector<int> finalAssignments;

    int initCheck = 0;

    cout << "[RIDESHARING] \033[0;35m ILP solve no of taxis " << list_Of_Trips.size() << " no of requests " << list_Of_Requests.size() << endl;
    if(list_Of_Trips.size() == 0) {
        return finalAssignments;
    }
    for (auto& j: list_Of_Trips) {
        vector<Trip> trips = j.second ;
        for (int y = 0; y < trips.size(); y++) {
            initCheck++;
        }
    }

    //cout << "initcheck is " << initCheck << endl;
    if(initCheck > 0) {
        int noOfVariables = 0;
        vector<double> objectiveFunction;

        //Build objective function  
        for (auto& j: list_Of_Trips) {
            vector<Trip> trips = j.second ;
            for (int y = 0; y < trips.size(); y++) {
                objectiveFunction.push_back(trips[y].getTripCost());
                noOfVariables++;
            }
        }

        for (int i=0;  i < list_Of_Requests.size(); i++) {
            objectiveFunction.push_back(1000.00);
            noOfVariables++;
        }
        // End buidling objective function 
        cout << "[RIDESHARING] \033[0;35m ILP solve : No. of Variables " << noOfVariables << endl;

        Model::t M = new Model("milo1"); auto _M = finally([&]() { M->dispose(); });
        Variable::t x = M->variable("x", noOfVariables, Domain::integral(Domain::greaterThan(0.0)));

        // Build constraint one
        int count = 0;
        int oneCount = 1;
        for (auto& l: list_Of_Trips) {
            int localCount = 0;
            vector<double> constraintOneFunction;
            for (auto& z: list_Of_Trips) {
                if(count == localCount) {
                    vector<Trip> trips = z.second ;
                    for (int y = 0; y < trips.size(); y++) {
                        constraintOneFunction.push_back(1.0);
                    }
                    localCount++;
                } else {
                    vector<Trip> trips = z.second ;
                    for (int y = 0; y < trips.size(); y++) {
                        constraintOneFunction.push_back(0.0);
                    }
                    localCount++;
                }
            }
            count++;

            for (int i=0;  i < list_Of_Requests.size(); i++) {
                constraintOneFunction.push_back(0.0);
            }

            double p1[constraintOneFunction.size()];
            for(int yy = 0; yy < constraintOneFunction.size(); yy++) {
                p1[yy] = constraintOneFunction[yy];
            }

            double *pp1 = p1;
            auto v = new ndarray<double,1>(pp1, shape(constraintOneFunction.size()));
    
            IntPtr pShared(v);
            auto a1 = pShared;
        
            M->constraint("c"+to_string(oneCount), Expr::dot(a1, x), Domain::lessThan(1.0));
            a1 = NULL;
            oneCount++;
        }
        // End buidling constraint one 

        // Build constraint two 
        int twoCount = 1 ;
        for (int q = 0; q < list_Of_Requests.size(); q++) {
            vector<double> constraintTwoFunction;
            for (auto& j: list_Of_Trips) {
                vector<Trip> trips = j.second ;
                for (int y = 0; y < trips.size(); y++) {
                    int check = 0;
                    for (int p = 0; p < trips[y].getTripPassengers().size(); p++) {
                        if(list_Of_Requests[q].getRequestID() == trips[y].getTripPassengers()[p].getRequestID()){
                            check = 1;
                            break;
                        }
                    }
                    if (check) {
                             constraintTwoFunction.push_back(1.0);
                    } else {
                            constraintTwoFunction.push_back(0.0);
                    }
                }
            }

            for (int i=0;  i < list_Of_Requests.size(); i++) {
                if (list_Of_Requests[i].getRequestID() == list_Of_Requests[q].getRequestID()){
                    constraintTwoFunction.push_back(1.0);
                 } else{
                    constraintTwoFunction.push_back(0.0);
                } 
            }

            double p2[constraintTwoFunction.size()];
            for(int yy = 0; yy < constraintTwoFunction.size(); yy++) {
                p2[yy] = constraintTwoFunction[yy];
            }

            double *pp2 = p2;
            auto v = new ndarray<double,1>(pp2, shape(constraintTwoFunction.size()));
    
            IntPtr pShared(v);
            auto a2 = pShared;
            M->constraint("d"+to_string(twoCount), Expr::dot(a2, x), Domain::equalsTo(1.0));
            twoCount++;
        }

        //End buidling constraint two
    
        //set the objective function values
        double o[objectiveFunction.size()];
        for(int yy = 0; yy < objectiveFunction.size(); yy++) {
            o[yy] = objectiveFunction[yy];
        }

        double *oo = o;
        auto v = new ndarray<double,1>(o, shape(objectiveFunction.size()));
    
        IntPtr pShared(v);
        auto c = pShared;
    

        // Set max solution time
        M->setSolverParam("mioMaxTime", 60.0);
        // Set max relative gap (to its default value)
        M->setSolverParam("mioTolRelGap", 1e-4);
        // Set max absolute gap (to its default value)
        M->setSolverParam("mioTolAbsGap", 0.0);
    
        // Set the objective function to (c^T * x)
        M->objective("obj", ObjectiveSense::Minimize, Expr::dot(c, x));
    
        // Solve the proble0
        M->solve();
    
         // Get the solution values
        auto sol = x->level();
        for(int k = 0;  k < noOfVariables; k++){
            finalAssignments.push_back((*sol)[k]);
        }

         return finalAssignments;
    } else {
        return finalAssignments;
    }  
}


//the function to solve the ILP for rebalancing
vector<int> MosekSolution::solveForRebalancing(map<int, vector<Trip> > list_Of_Trips, vector<Request> list_Of_Requests, vector<Ride> unassignedRides) {
   
    vector<int> finalAssignments;

    int initCheck = 0;

    if(list_Of_Trips.size() == 0) {
        return finalAssignments;
    }
    for (auto& j: list_Of_Trips) {
        vector<Trip> trips = j.second ;
        for (int y = 0; y < trips.size(); y++) {
            initCheck++;
        }
    }

    if(initCheck > 0) {
        int noOfVariables = 0;
        vector<double> objectiveFunction;
        //Build objective function  
        for (auto& j: list_Of_Trips) {
            vector<Trip> trips = j.second ;
            for (int y = 0; y < trips.size(); y++) {
                objectiveFunction.push_back(trips[y].getTripCost());
                noOfVariables++;
            }
        }

        for (int i=0;  i < list_Of_Requests.size(); i++) {
            objectiveFunction.push_back(1000.00);
            noOfVariables++;
        }

        // End buidling objective function 

        Model::t M = new Model("milo1"); auto _M = finally([&]() { M->dispose(); });
        Variable::t x = M->variable("x", noOfVariables, Domain::integral(Domain::greaterThan(0.0)));

        // Build constraint one  

        int count = 0;
        int oneCount = 1;
        for (auto& l: list_Of_Trips) {
            int localCount = 0;
            vector<double> constraintOneFunction;
            for (auto& z: list_Of_Trips) {
                if(count == localCount) {
                    vector<Trip> trips = z.second ;
                    for (int y = 0; y < trips.size(); y++) {
                        constraintOneFunction.push_back(1.0);
                    }
                    localCount++;
                } else {
                    vector<Trip> trips = z.second ;
                    for (int y = 0; y < trips.size(); y++) {
                        constraintOneFunction.push_back(0.0);
                    }
                    localCount++;
                }
            }
            count++;

            for (int i=0;  i < list_Of_Requests.size(); i++) {
                constraintOneFunction.push_back(0.0);
            }

            double p1[constraintOneFunction.size()];
            for(int yy = 0; yy < constraintOneFunction.size(); yy++) {
                p1[yy] = constraintOneFunction[yy];
            }

            double *pp1 = p1;
            auto v = new ndarray<double,1>(pp1, shape(constraintOneFunction.size()));
    
            IntPtr pShared(v);
            auto a1 = pShared;
        
            M->constraint("c"+to_string(oneCount), Expr::dot(a1, x), Domain::lessThan(1.0));
            a1 = NULL;
            oneCount++;
         }
    
        // End buidling constraint one 

        // Build constraint two 
        int twoCount = 1 ;
        for (int q = 0; q < list_Of_Requests.size(); q++) {
            vector<double> constraintTwoFunction;
            for (auto& j: list_Of_Trips) {
                vector<Trip> trips = j.second ;
                for (int y = 0; y < trips.size(); y++) {
                    int check = 0;
                    for (int p = 0; p < trips[y].getTripPassengers().size(); p++) {
                        if(list_Of_Requests[q].getRequestID() == trips[y].getTripPassengers()[p].getRequestID()){
                            check = 1;
                            break;
                        }
                    }
                    if (check) {
                             constraintTwoFunction.push_back(1.0);
                    } else {
                            constraintTwoFunction.push_back(0.0);
                    }
                }
            }

            for (int i=0;  i < list_Of_Requests.size(); i++) {
                if (list_Of_Requests[i].getRequestID() == list_Of_Requests[q].getRequestID()){
                    constraintTwoFunction.push_back(1.0);
                 } else{
                    constraintTwoFunction.push_back(0.0);
                } 
            }

            double p2[constraintTwoFunction.size()];
            for(int yy = 0; yy < constraintTwoFunction.size(); yy++) {
                p2[yy] = constraintTwoFunction[yy];
            }

            double *pp2 = p2;
            auto v = new ndarray<double,1>(pp2, shape(constraintTwoFunction.size()));
    
            IntPtr pShared(v);
            auto a2 = pShared;
            M->constraint("d"+to_string(twoCount), Expr::dot(a2, x), Domain::equalsTo(1.0));
            twoCount++;
        }

        //End buidling constraint two

        //build constraint three 
        if (list_Of_Requests.size() <= unassignedRides.size()) {
            vector<double> constraintThreeFunction;

            //Build objective function  
            for (auto& j: list_Of_Trips) {
                vector<Trip> trips = j.second ;
                for (int y = 0; y < trips.size(); y++) {
                    constraintThreeFunction.push_back(0.0);
                }
            }

            for (int i=0;  i < list_Of_Requests.size(); i++) {
                constraintThreeFunction.push_back(1.0);
            }

            double p3[constraintThreeFunction.size()];
            for(int yy = 0; yy < constraintThreeFunction.size(); yy++) {
                p3[yy] = constraintThreeFunction[yy];
            }

            double *pp3 = p3;
            auto v = new ndarray<double,1>(pp3, shape(constraintThreeFunction.size()));
    
            IntPtr pShared(v);
            auto a3 = pShared;

            M->constraint("e1", Expr::dot(a3, x), Domain::equalsTo(0.0));

        } else{
            
            vector<double> constraintThreeFunction;

            //Build objective function  
            for (auto& j: list_Of_Trips) {
                vector<Trip> trips = j.second ;
                for (int y = 0; y < trips.size(); y++) {
                    constraintThreeFunction.push_back(1.0);
                }
            }

            for (int i=0;  i < list_Of_Requests.size(); i++) {
                constraintThreeFunction.push_back(0.0);
            }

            double p3[constraintThreeFunction.size()];
            for(int yy = 0; yy < constraintThreeFunction.size(); yy++) {
                p3[yy] = constraintThreeFunction[yy];
            }

            double *pp3 = p3;
            auto v = new ndarray<double,1>(pp3, shape(constraintThreeFunction.size()));
    
            IntPtr pShared(v);
            auto a3 = pShared;
            M->constraint("e1", Expr::dot(a3, x), Domain::equalsTo(unassignedRides.size()));
        }
        //End buidling constraint three
    
        //set the objective function values
        double o[objectiveFunction.size()];
        for(int yy = 0; yy < objectiveFunction.size(); yy++) {
            o[yy] = objectiveFunction[yy];
        }

        double *oo = o;
        auto v = new ndarray<double,1>(o, shape(objectiveFunction.size()));
    
        IntPtr pShared(v);
        auto c = pShared;
    
        // Set max solution time
        M->setSolverParam("mioMaxTime", 60.0);
        // Set max relative gap (to its default value)
        M->setSolverParam("mioTolRelGap", 1e-4);
        // Set max absolute gap (to its default value)
        M->setSolverParam("mioTolAbsGap", 0.0);
    
        // Set the objective function to (c^T * x)
        M->objective("obj", ObjectiveSense::Minimize, Expr::dot(c, x));
    
        // Solve the proble0
        M->solve();
    
         // Get the solution values
        auto sol = x->level();
   
        for(int k = 0;  k < noOfVariables; k++){
            finalAssignments.push_back((*sol)[k]);
        }

         return finalAssignments;
    } else {
        return finalAssignments;
    }  
}

