/****************************************************************************/
/// @file    Edge.h
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

#ifndef EDGE_H
#define EDGE_H

#include <vector>
#include <omnetpp.h>

#include "router/Node.h"
#include "router/EdgeCosts.h"

namespace VENTOS {

class Lane
{
public:
    std::string id;
    double speed;
    double length;
    std::vector<int> greenPhases; //Set of traffic-light phases that give this lane a green light
    Lane(std::string i, double s, double l);
};

class Edge
{
public:
    Edge(std::string idVal, Node* fromVal, Node* toVal, int priorityVal, std::vector<Lane*> lanesVec);
    double getCost();

public:
    int debugLevel;

    //Node variables
    std::string id;
    Node* from;
    Node* to;
    int priority;
    int numLanes;

    //Weighting variables
    double speed;
    double length;
    std::vector<Lane*> lanes;

    //Pathing variables
    EdgeCosts travelTimes;
    bool visited;
    double curCost;
    Edge* best;

    //Removal algorithm
    bool disabled;
};

std::ostream& operator<<(std::ostream& os, Edge &rhs);

}

#endif
