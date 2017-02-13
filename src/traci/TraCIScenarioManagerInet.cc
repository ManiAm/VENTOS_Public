/****************************************************************************/
/// @file    TraCIScenarioManagerInet.h
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

#include "mobility/Coord.h"

#ifdef WITH_INET

#define WITH_TRACI
#include "mobility/single/TraCIMobility.h"

void ifInetTraCIMobilityCallPreInitialize(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, double angle) {
	::TraCIMobility* inetmm = dynamic_cast< ::TraCIMobility*>(mod);
	if (!inetmm) return;
	inetmm->preInitialize(nodeId, ::Coord(position.x, position.y), road_id, speed, angle);
}


void ifInetTraCIMobilityCallNextPosition(cModule* mod, const Coord& p, const std::string& edge, double speed, double angle) {
	::TraCIMobility *inetmm = dynamic_cast< ::TraCIMobility*>(mod);
	if (!inetmm) return;
	inetmm->nextPosition(::Coord(p.x, p.y), edge, speed, angle);
}

#else // not WITH_INET

void ifInetTraCIMobilityCallPreInitialize(omnetpp::cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, double angle) {
	return;
}


void ifInetTraCIMobilityCallNextPosition(omnetpp::cModule* mod, const Coord& p, const std::string& edge, double speed, double angle) {
	return;
}

#endif // WITH_INET
