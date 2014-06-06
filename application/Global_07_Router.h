#ifndef ROUTER_H
#define ROUTER_H

#include<vector>
#include<list>
#include<fstream>

#include <BaseModule.h>
#include "NodeEdge.h"
#include "ApplV_02_Beacon.h"

using namespace std;

class Router : public BaseModule    //Responsible for routing cars in our system.  Should only be one of these.
{
protected:
    //Routing code
    vector<Edge> edges;
    vector<Node> nodes;
    bool enableRouting;

    double getEdgeMeanSpeed(Edge* edge);    //Get the mean speed of all lanes on an edge
    void updateWeights();   //Recalculates edge weights
    double lastUpdateTime;  //The last time updateWeights() ran
    int recalculateCount;   //How many times updateWeights() has ran

    void reset();   //Resets the pathing info on all nodes
    list<string> getRoute(string begin, string end); // Returns a list of edges between origin and destination,
                                                     // or an empty list if they're not connected
    //Internal functions
    virtual void initialize(int);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);
    SystemMsg* prepareSystemMsg();

    //Message passing stuff
    cModule *nodePtr;               // pointer to the Node
    mutable TraCI_Extend* manager;  //Link to TraCI
    simsignal_t Signal_system;      //Receives signals to here

public:
    friend ostream& operator<<(ostream& os, Router &rhs);
    Router();
    ~Router();
};

#endif
