#ifndef ROUTER_H
#define ROUTER_H

#include<vector>
#include<list>
#include<fstream>

#include <iostream>  // For testing - remove
#include <iomanip>   // For operator<< code
#include <queue>     // For pathing
#include <climits>   // For INT_MAX
#include <algorithm> // For sort
#include <sstream>   // For getline() in stringToList

#include <BaseModule.h>
#include "NodeEdge.h"
#include "ApplV_02_Beacon.h"
#include "Global_01_TraCI_Extend.h"

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

using namespace rapidxml;
using namespace std;

namespace VENTOS {

class Router : public BaseModule    //Responsible for routing cars in our system.  Should only be one of these.
{
public:
    ~Router();
    friend ostream& operator<<(ostream& os, Router &rhs);

protected:
    //Routing code
    vector<Edge> edges;         //Edges in our graph
    vector<Node> nodes;         //Nodes in our graph
    bool enableRouting;         //If false, runs no code
    double lastUpdateTime;      //The last time updateWeights() ran
    int recalculateCount;       //How many times updateWeights() has ran

    double getEdgeMeanSpeed(Edge* edge);    //Get the mean speed of all lanes on an edge
    void updateWeights();       //Recalculates edge weights
    void reset();   //Resets the pathing info on all nodes
    list<string> getRoute(string begin, string end);   //Returns a list of edges between origin and destination,
                                                       //or an empty list if they're not connected
    //Internal functions
    virtual void initialize(int);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);
    SystemMsg* prepareSystemMsg();

    //Message passing stuff
    cModule *nodePtr;               // pointer to the Node
    mutable TraCI_Extend* manager;  //Link to TraCI
    simsignal_t Signal_system;      //Receives signals to here
};
}

#endif
