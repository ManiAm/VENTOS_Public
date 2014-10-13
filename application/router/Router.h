#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <list>
#include <fstream>
#include <BaseModule.h>

#include "Node.h"
#include "Edge.h"
#include "TrafficLight.h"
#include "Net.h"
#include "Hypertree.h"

#include "ApplV_02_Beacon.h"


using namespace std;

namespace VENTOS {

class Net;

class Router : public BaseModule    //Responsible for routing cars in our system.  Should only be one of these.
{
public:
    ~Router();
    friend ostream& operator<<(ostream& os, Router &rhs);

protected:
    Net* net;
    map<string, Hypertree*> hypertreeMemo;

    bool enableRouting; //If false, runs no code

    double leftTurnCost, rightTurnCost, straightCost, uTurnCost, TLLookahead;

    double junctionCost(double time, Edge* start, Edge* end);       //If it's a TL, returns the time spent waiting.  If not, returns turnTypeCost
    double turnTypeCost(Edge* start, Edge* end);                    //Returns the turn penalty on an intersection
    double timeToPhase(TrafficLight* tl, double time, int phase);   //Returns how long we must wait for a given phase at the given time
    int nextAcceptingPhase(double time, Edge* start, Edge* end);    //Returns the next phase allowing movement from start to end at the given time
    vector<int>* TLTransitionPhases(Edge* start, Edge* end);        //Returns a vector of phases allowing movement from start to end

    double getEdgeMeanSpeed(Edge* edge);    //Get the mean speed of all lanes on an edge

    int timePeriodMax;     //Max time for hypertrees
    Hypertree* buildHypertree(int startTime, Node* destination);    //Builds a hypertree to the destination, bounded between the start time and timePeriodMax;
    list<string> getRoute(Edge* origin, Node* destination, string vName);      //Returns a list of edges between origin and destination,
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
