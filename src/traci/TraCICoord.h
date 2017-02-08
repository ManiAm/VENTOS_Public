/****************************************************************************/
/// @file    TraCICoord.h
/// @author  Christoph Sommer <mail@christoph-sommer.de>
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

#ifndef VEINS_MOBILITY_TRACI_TRACICOORD_H_
#define VEINS_MOBILITY_TRACI_TRACICOORD_H_

namespace VENTOS {

// Coord equivalent for storing TraCI coordinates
struct TraCICoord
{
    double x;
    double y;
    double z;

	TraCICoord() : x(0.0), y(0.0), z(0.0) {}
    TraCICoord(double x, double y) : x(x), y(y), z(0.0) {}
	TraCICoord(double x, double y, double z) : x(x), y(y), z(z) {}

	bool operator == (const TraCICoord &rhs)
	{
	    return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
	}
};

}

#endif
