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

#include "Router.h"
#include "Vehicle.h"
#include "Histogram.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

namespace VENTOS {

class Connection
{
public:
    std::string from;
    std::string to;
    int fromLane;
    int toLane;
    std::string TLid;
    int linkIndex;
    char dir;
    char state;

    Connection(std::string from, std::string to, int fromLane, int toLane, std::string TLid, int linkIndex, char dir, char state):
        from(from), to(to), fromLane(fromLane), toLane(toLane), TLid(TLid), linkIndex(linkIndex), dir(dir), state(state)
    {}
};

class Net
{
public:
    std::map<std::string, TrafficLightRouter*> TLs;
    std::map<std::string, Edge*> edges;
    std::map<std::string, Node*> nodes;
    std::map<std::string, Vehicle*> vehicles;
    std::map<std::string, std::vector<Connection*> > connections;

    std::map<std::string, std::vector<int>* >* transitions;  //Given a pair of edge IDs concatenated, returns a vector of TL phases that allow movement between them
    std::map<std::string, char>* turnTypes;           //Given a pair of edge IDs concatenated, returns the turn type between those two

    cModule* routerModule;

    Net(std::string fileName, cModule* router);
    ~Net();
};

}

#endif
