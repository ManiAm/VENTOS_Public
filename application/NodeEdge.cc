#include<iostream>
#include<climits>   //For INT_MAX
#include<iomanip>   //For setw()

#include "NodeEdge.h"

Node::Node(string idVal, double xVal, double yVal, string typeVal) // Build a node
{
    id = idVal;
    x = xVal;
    y = yVal;
    type = typeVal;
    visited = 0;
    curCost = INT_MAX;
    best = NULL;
}

bool Node::operator<(Node &rhs)   // For priority queue sorting
{
    return (this->curCost < rhs.curCost);
}


bool Node::operator>(Node &rhs)   // For priority queue sorting
{
    return (this->curCost > rhs.curCost);
}


ostream& operator<<(ostream& os, Node &rhs) // Print a node
{
    os << "id: "<< setw(3) << left << rhs.id <<
         " x: " << setw(5) << left << rhs.x <<
         " y: " << setw(5) << left << rhs.y <<
         " type: " << rhs.type;
    for(unsigned int i = 0; i < rhs.edges.size(); i++)
        os << endl << "    Connects to node " << rhs.edges[i]->to->id << " via " <<rhs.edges[i]->id;

    return os;
}

Lane::Lane(string i, double l)
{
    id = i;
    length = l;
}

Edge::Edge(string idVal, Node* fromVal, Node* toVal, int priorityVal, int numLanesVal, double speedVal) // This will likely grow
{
    id = idVal;
    from = fromVal;
    to = toVal;
    priority = priorityVal;
    numLanes = numLanesVal;
    speed = speedVal;
}

double Edge::updateLength()
{
    double ret = 0;
    for(vector<Lane>::iterator l = lanes.begin(); l != lanes.end(); l++)
        ret += (*l).length;
    ret = ret / lanes.size();
    length = ret;
    return ret;
}

double Edge::getCost()  // This will likely be more complex once weights are implemented
{
    return origCost;
    //return manager->getEdgeLength(edgeID) / manager->getEdgeCost(edgeID);
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

EdgePair::EdgePair(Edge* e, Node* n)    // Used for pathing.
{
    edge = e;
    node = n;
}

bool EdgePair::operator<(EdgePair &rhs)
{
    return this->node->curCost < rhs.node->curCost; // If the cost is less, return true
}

bool EdgePair::operator>(EdgePair &rhs)
{
    return this->node->curCost > rhs.node->curCost; // If the cost is less, return true
}
bool NodeIDSort(const Node &n1, const Node &n2)
{
    return n1.id < n2.id;
}

bool EdgeIDSort(const Edge &e1, const Edge &e2)
{
    return e1.id < e2.id;
}




