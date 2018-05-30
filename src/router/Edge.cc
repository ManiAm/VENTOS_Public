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

Lane::Lane(std::string id, double speed, double length):
                                  id(id), speed(speed), length(length){}

Edge::Edge(std::string id, Node* from, Node* to, int priority, std::vector<Lane*> lanes):
                                  id(id), from(from), to(to), priority(priority), lanes(lanes)
{
    debugLevel = omnetpp::getSimulation()->getSystemModule()->par("debugLevel").intValue();

    numLanes = lanes.size();
    speed = 0;
    length = 0;
    for(int i = 0; i < numLanes; i++)
    {
        speed += lanes[i]->speed;
        length += lanes[i]->length;
    }
    speed /= numLanes;
    length /= numLanes;

    visited = 0;
    curCost = 100000000;
    best = NULL;
    disabled = false;
}

double Edge::getCost()
{
    if(travelTimes.average > 0)
        return travelTimes.average;

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
    {
        std::cout << "Using length / speed for cost of edge " << id << std::endl;
        std::cout.flush();
    }

    return length / speed;
}

std::ostream& operator<<(std::ostream& os, Edge &rhs) // Print an Edge
{
    os << "id: "         << std::setw(3) << std::left << rhs.id
            << " from: "      << std::setw(3) << std::left << rhs.from->id
            << " to: "        << std::setw(3) << std::left << rhs.to->id
            << " priority: "  << std::setw(4) << std::left << rhs.priority
            << " numLanes: "  << std::setw(3) << std::left << rhs.numLanes
            << " speed: "     << std::setw(5) << std::left << rhs.speed;

    return os;
}

}
