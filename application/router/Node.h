#ifndef NODE_H
#define NODE_H

#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

namespace VENTOS {

class Edge;

class TrafficLight;

class Node  //Node in our graph, points to a bunch of other edges and nodes
{
public:
    //Node variables
    vector<Edge*> outEdges;
    vector<Edge*> inEdges;
    string id;
    double x;
    double y;
    string type;
    TrafficLight* tl;
    vector<string>* incLanes;

    bool operator==(const Node& rhs);
    Node(string idVal, double xVal, double yVal, string typeVal, vector<string>* incLanesVal, TrafficLight* tlVal);
};
bool NodeIDSort(const Node* n1, const Node* n2);
ostream& operator<<(ostream& os, Node &rhs);

}

#endif
