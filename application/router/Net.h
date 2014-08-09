#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm> // For sort

#include "Router.h"
#include "Global_01_TraCI_Extend.h"
#include "Histogram.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

using namespace std;
using namespace rapidxml;

namespace VENTOS {

class Net
{
public:
    vector<TrafficLight*> TLs;  //TLs in our graph
    vector<Edge*> edges;        //Edges in our graph
    vector<Node*> nodes;        //Nodes in our graph
    map<string, vector<int>* >* transitions;  //Given a pair of edge IDs concatenated, returns a vector of TL phases that allow movement between them
    map<string, char>* turnTypes;           //Given a pair of edge IDs concatenated, returns the turn type between those two

    Net(string fileName);
    ~Net();
};
}

#endif
