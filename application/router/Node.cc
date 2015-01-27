
#include "Node.h"

namespace VENTOS {

bool Node::operator==(const Node& rhs)
{
    return this->id == rhs.id;
}

Node::Node(string id, double x, double y, string type, vector<string>* incLanes, TrafficLightRouter* tl): // Build a node
        id(id), x(x), y(y), type(type), incLanes(incLanes), tl(tl){}

ostream& operator<<(ostream& os, Node &rhs) // Print a node
{
    os << "id: "<< setw(3) << left << rhs.id <<
         " x: " << setw(5) << left << rhs.x <<
         " y: " << setw(5) << left << rhs.y <<
         " type: " << rhs.type;
    return os;
}

}
