#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm> // For sort
#include <map>

#include "Router.h"
#include "Vehicle.h"
#include "Histogram.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

namespace VENTOS {

class Connection
{
public:
    string from;
    string to;
    int fromLane;
    int toLane;
    string TLid;
    int linkIndex;
    char dir;
    char state;

    Connection(string from, string to, int fromLane, int toLane, string TLid, int linkIndex, char dir, char state):
        from(from), to(to), fromLane(fromLane), toLane(toLane), TLid(TLid), linkIndex(linkIndex), dir(dir), state(state)
    {}
};

class Net
{
public:

    map<string, TrafficLightRouter*> TLs;
    map<string, Edge*> edges;
    map<string, Node*> nodes;
    map<string, Vehicle*> vehicles;
    map<string, vector<Connection*> > connections;

    map<string, vector<int>* >* transitions;  //Given a pair of edge IDs concatenated, returns a vector of TL phases that allow movement between them
    map<string, char>* turnTypes;           //Given a pair of edge IDs concatenated, returns the turn type between those two

    cModule* routerModule;

    Net(string fileName, cModule* router);
    ~Net();
};

}

#endif
