/****************************************************************************/
/// @file    Node.cc
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

#include "router/Node.h"

namespace VENTOS {

/*
bool Node::operator==(const Node& rhs)
{
    return this->id == rhs.id;
}*/

Node::Node(std::string id, double x, double y, std::string type, std::vector<std::string>* incLanes, TrafficLightRouter* tl): // Build a node
                  id(id), x(x), y(y), type(type), incLanes(incLanes), tl(tl){}

std::ostream& operator<<(std::ostream& os, Node &rhs) // Print a node
{
    os << "id: "<< std::setw(3) << std::left << rhs.id <<
         " x: " << std::setw(5) << std::left << rhs.x <<
         " y: " << std::setw(5) << std::left << rhs.y <<
         " type: " << rhs.type;
    return os;
}

}
