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
#include "PathingData.h"

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

    bool enableRouting;         //If false, runs no code
    double lastUpdateTime;      //The last time updateWeights() ran
    int timePeriod;

    double leftTurnCost, rightTurnCost, straightCost, uTurnCost, TLLookahead;

    double turnTypeCost(string key);
    double TLCost(double time, Edge* start, Edge* end);
    vector<int>* TLTransitionPhases(Edge* start, Edge* end);
    int currentPhase(TrafficLight* tl, double time, double* timeRemaining = NULL);
    double timeToPhase(TrafficLight* tl, double time, int phase);
    int nextAcceptingPhase(double time, Edge* start, Edge* end);

    double getEdgeMeanSpeed(Edge* edge);    //Get the mean speed of all lanes on an edge
    void updateWeights();                   //Recalculates edge weights
    void reset();                           //Resets the pathing info on all nodes
    Hypertree* buildHypertree(int startTime, int endTime, string destinationNodeID);
    list<string> getRoute(string begin, string end, string vName);      //Returns a list of edges between origin and destination,
                                                                        //or an empty list if they're not connected

    //Internal functions
    virtual void initialize(int);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);
    SystemMsg* prepareSystemMsg();

    //Message passing stuff
    cModule *nodePtr;               // pointer to the Node
    mutable TraCI_Extend* TraCI;  //Link to TraCI
    simsignal_t Signal_system;      //Receives signals to here
};

string key(Node* n1, Node* n2, int time);

}

#endif
