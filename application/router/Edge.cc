#include "Edge.h"

namespace VENTOS {

Lane::Lane(string id, double speed, double length):
        id(id), speed(speed), length(length){}

Edge::Edge(string id, Node* from, Node* to, int priority, vector<Lane*>* lanes, Histogram* travelTimes):
        id(id), from(from), to(to), priority(priority), lanes(lanes), travelTimes(travelTimes)
{
    numLanes = lanes->size();
    double speedVal = 0;
    double lengthVal = 0;
    for(int i = 0; i < numLanes; i++)
    {
        speedVal += (*lanes)[i]->speed;
        lengthVal += (*lanes)[i]->length;
    }
    speed = speedVal / numLanes;
    length = lengthVal / numLanes;

    visited = 0;
    curCost = 100000000;
    best = NULL;
}

double Edge::getCost()  // This will likely be more complex once weights are implemented
{
    if(travelTimes->average != 0)
        return travelTimes->average;
    cout << "using length / speed" << endl;
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

}

