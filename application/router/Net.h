#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm> // For sort
#include <map>

#include "Router.h"
#include "Global_04_Statistics.h"
#include "Histogram.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

namespace VENTOS {

class Net
{
public:

    map<string, TrafficLight*> TLs;
    map<string, Edge*> edges;
    map<string, Node*> nodes;

    map<string, vector<int>* >* transitions;  //Given a pair of edge IDs concatenated, returns a vector of TL phases that allow movement between them
    map<string, char>* turnTypes;           //Given a pair of edge IDs concatenated, returns the turn type between those two

    cModule* routerMod;

    Net(string fileName, cModule* router);
    ~Net();
};

}

#endif
