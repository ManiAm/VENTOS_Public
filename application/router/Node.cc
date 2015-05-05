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

#include "Node.h"

namespace VENTOS {

bool Node::operator==(const Node& rhs)
{
    return this->id == rhs.id;
}

Node::Node(string id, double x, double y, string type, vector<string>* incLanes, TrafficLightRouter* tl): // Build a node
                  id(id), x(x), y(y), type(type), incLanes(incLanes), tl(tl){}

ostream& operator<<(ostream& os, Node &rhs) // Print a node
{
    os << "id: "<< setw(3) << left << rhs.id <<
         " x: " << setw(5) << left << rhs.x <<
         " y: " << setw(5) << left << rhs.y <<
         " type: " << rhs.type;
    return os;
}

}
