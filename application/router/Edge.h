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
    double length;
    double speed;
    Lane(string i, double s, double l);
};

class Edge
{
public:
    //Pathing variables
    Histogram* travelTimes;
    bool visited;
    double curCost;
    Edge* best;

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
    vector<Lane*>* lanes;
    double getCost();

    Edge(string idVal, Node* fromVal, Node* toVal, int priorityVal, vector<Lane*>* lanesVec, Histogram* hist);
};
bool EdgeIDSort(const Edge* n1, const Edge* n2);
ostream& operator<<(ostream& os, Edge &rhs);

}

#endif
