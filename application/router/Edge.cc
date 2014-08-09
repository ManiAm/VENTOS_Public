#include "Edge.h"

namespace VENTOS {

string laneToEdgeID(string s)
{
    for(int i = s.length() - 1; i >= 0; i--)
        if(s[i] == '_')
            return s.substr(0,i);
    return s;
}

Lane::Lane(string i, double s, double l)
{
    id = i;
    length = l;
    speed = s;
}

Edge::Edge(string idVal, Node* fromVal, Node* toVal, int priorityVal, vector<Lane*>* lanesVec, Histogram* hist)
{
    id = idVal;
    from = fromVal;
    to = toVal;
    priority = priorityVal;
    numLanes = lanesVec->size();
    double speedVal = 0;
    double lengthVal = 0;
    for(int i = 0; i < numLanes; i++)
    {
        speedVal += (*lanesVec)[i]->speed;
        lengthVal += (*lanesVec)[i]->length;
    }
    speed = speedVal / numLanes;
    length = lengthVal / numLanes;
    travelTimes = hist;
    lanes = lanesVec;

    visited = 0;
    curCost = 100000000;
    best = NULL;
}

double Edge::getCost()  // This will likely be more complex once weights are implemented
{
    if(travelTimes->average != 0)
        return travelTimes->average;
    return length / speed;
}


ostream& operator<<(ostream& os, Edge &rhs) // Print an Edge
{
    os << "id: "         << setw(3) << left << rhs.id
       << " from: "      << setw(3) << left << rhs.from->id
       << " to: "        << setw(3) << left << rhs.to->id
       << " priority: "  << setw(4) << left << rhs.priority
       << " numLanes: "  << setw(3) << left << rhs.numLanes
       << " speed: "     << setw(5) << left << rhs.speed;
    return os;
}


bool EdgeIDSort(const Edge* e1, const Edge* e2)
{
    return e1->id < e2->id;
}

}

