#include "Node.h"

namespace VENTOS {

bool Node::operator==(const Node& rhs)
{
    return this->id == rhs.id;
}
Node::Node(string idVal, double xVal, double yVal, string typeVal, vector<string>* incLanesVal, TrafficLight* tlVal) // Build a node
{
    id = idVal;
    x = xVal;
    y = yVal;
    type = typeVal;
    incLanes = incLanesVal;
    tl = tlVal;
}

/*
ostream& operator<<(ostream& os, Node &rhs) // Print a node
{
    os << "id: "<< setw(3) << left << rhs.id <<
         " x: " << setw(5) << left << rhs.x <<
         " y: " << setw(5) << left << rhs.y <<
         " type: " << rhs.type;
    for(unsigned int i = 0; i < rhs.edges.size(); i++)
        os << endl << "    Connects to node " << rhs.edges[i]->to->id << " via " <<rhs.edges[i]->id;

    return os;
}*/

bool NodeIDSort(const Node* n1, const Node* n2)
{
    return n1->id < n2->id;
}

}

