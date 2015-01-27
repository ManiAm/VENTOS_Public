#ifndef ROUTER_H
#define ROUTER_H

#include <BaseModule.h>
#include <vector>
#include <list>
#include <fstream>

#include "Node.h"
#include "Edge.h"
#include "TrafficLightRouter.h"
#include "Net.h"
#include "Hypertree.h"

#include "ApplV_02_Beacon.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & >( (std::ostringstream() << std::dec << x ) ).str()

using namespace std;

namespace VENTOS {

class Node;
class Edge;
class TrafficLightRouter;
class Net;
class Hypertree;

class Router : public BaseModule    //Responsible for routing cars in our system.  Should only be one of these.
{
public:
    ~Router();

    int createTime;
    int currentVehicleCount;
    int totalVehicleCount;
    double nonReroutingVehiclePercent;
    vector<string>* nonReroutingVehicles;
    Net* net;
    void sendRerouteSignal(string vid);     //Forces a vehicle to reroute
    bool UseHysteresis;

protected:
    map<string, Hypertree*> hypertreeMemo;

    //Vehicle stuff
    bool collectVehicleTimeData;
    map<string, int> vehicleTravelTimes;
    ofstream vehicleTravelTimesFile;

    bool enableRouting; //If false, runs no code
    double leftTurnCost, rightTurnCost, straightCost, uTurnCost, TLLookahead;

    double junctionCost(double time, Edge* start, Edge* end);       //If it's a TL, returns the time spent waiting.  If not, returns turnTypeCost
    double turnTypeCost(Edge* start, Edge* end);                    //Returns the turn penalty on an intersection
    double timeToPhase(TrafficLightRouter* tl, double time, int phase);   //Returns how long we must wait for a given phase at the given time
    int nextAcceptingPhase(double time, Edge* start, Edge* end);    //Returns the next phase allowing movement from start to end at the given time
    vector<int>* TLTransitionPhases(Edge* start, Edge* end);        //Returns a vector of phases allowing movement from start to end

    int timePeriodMax;     //Max time for hypertrees
    Hypertree* buildHypertree(int startTime, Node* destination);    //Builds a hypertree to the destination, bounded between the start time and timePeriodMax;
    list<string> getRoute(Edge* origin, Node* destination, string vName);       //Returns a list of edges between origin and destination,
                                                                                //or an empty list if they're not connected
    //Internal functions
    virtual void initialize(int);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);
    SystemMsg* prepareSystemMsg();

    //Message passing
    cModule *nodePtr;               // pointer to the Node
    mutable TraCI_Extend* TraCI;    //Link to TraCI
    simsignal_t Signal_system;      //Receives signals to here
};

string key(Node* n1, Node* n2, int time);

}

#endif
