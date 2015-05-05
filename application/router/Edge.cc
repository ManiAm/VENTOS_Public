/****************************************************************************/
/// @file    Edge.cc
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

#include "Edge.h"

namespace VENTOS {

Lane::Lane(string id, double speed, double length):
                  id(id), speed(speed), length(length){}

Edge::Edge(string id, Node* from, Node* to, int priority, vector<Lane*> lanes, LaneCosts* travelTimes):
                  id(id), from(from), to(to), priority(priority), lanes(lanes), travelTimes(travelTimes)
{
    numLanes = lanes.size();
    double speedVal = 0;
    double lengthVal = 0;
    for(int i = 0; i < numLanes; i++)
    {
        speedVal += lanes[i]->speed;
        lengthVal += lanes[i]->length;
    }
    speed = speedVal / numLanes;
    length = lengthVal / numLanes;

    visited = 0;
    curCost = 100000000;
    best = NULL;
    disabled = false;
}

double Edge::getCost()  // This will likely be more complex once weights are implemented
{
    if(travelTimes->average != 0)
        return travelTimes->average;
    cout << "using length / speed" << endl;
    return length / speed;
}

ostream& operator<<(ostream& os, Edge &rhs) // Print an Edge
{
    os << "id: "         << setw(3) << left << rhs.id
       << " from: "      << setw(3) << left << rhs.from->id
       << " to: "        << setw(3) << left << rhs.to->id
       << " priority: "  << setw(4) << left << rhs.priority
       << " numLanes: "  << setw(3) << left << rhs.numLanes
       << " speed: "     << setw(5) << left << rhs.speed;
    return os;
}

}

