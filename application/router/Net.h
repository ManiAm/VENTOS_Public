/****************************************************************************/
/// @file    Net.h
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  second author here
/// @date    August 2013
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

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
