#ifndef EDGE_H
#define EDGE_H

#include <vector>
#include <iostream>
#include <iomanip>

#include "Node.h"
#include "Histogram.h"

using namespace std;

namespace VENTOS {

class Lane
{
public:
    string id;
    double speed;
    double length;
    vector<int> greenPhases;
    Lane(string i, double s, double l);
};

class Edge
{
public:
    //Node variables
    string id;
    Node* from;
    Node* to;
    int priority;
    int numLanes;

    //Weighting variables
    double speed;
    double length;
    double origWeight;
    double lastWeight;
    vector<Lane*> lanes;
    double getCost();

    //Pathing variables
    Histogram* travelTimes;
    bool visited;
    double curCost;
    Edge* best;

    Edge(string idVal, Node* fromVal, Node* toVal, int priorityVal, vector<Lane*> lanesVec, Histogram* hist);
};

ostream& operator<<(ostream& os, Edge &rhs);

}

#endif
