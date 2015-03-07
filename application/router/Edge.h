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
#include <iostream>
#include <iomanip>

#include "Node.h"
#include "Histogram.h"

using namespace std;

namespace VENTOS {

class Lane
{
public:
    string id;
    double speed;
    double length;
    vector<int> greenPhases;
    Lane(string i, double s, double l);
};

class Edge
{
public:
    //Node variables
    string id;
    Node* from;
    Node* to;
    int priority;
    int numLanes;

    //Weighting variables
    double speed;
    double length;
    double origWeight;
    double lastWeight;
    vector<Lane*> lanes;
    double getCost();

    //Pathing variables
    Histogram* travelTimes;
    bool visited;
    double curCost;
    Edge* best;

    Edge(string idVal, Node* fromVal, Node* toVal, int priorityVal, vector<Lane*> lanesVec, Histogram* hist);
};

ostream& operator<<(ostream& os, Edge &rhs);

}

#endif
