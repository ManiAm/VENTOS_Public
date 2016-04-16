/****************************************************************************/
/// @file    TrafficLights.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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

#ifndef TRAFFICLIGHTS_H
#define TRAFFICLIGHTS_H

#include <02_LoopDetectors.h>
#include <unordered_map>

namespace VENTOS {

class TrafficLights : public LoopDetectors
{
public:
    virtual ~TrafficLights();
    virtual void initialize(int);
    virtual void finish();
    virtual void handleMessage(cMessage *);

protected:
    void virtual initialize_withTraCI();
    void virtual executeEachTimeStep();

protected:

    // list of all traffic lights in the network
    std::list<std::string> TLList;

    // list of all 'incoming lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::pair<int /*lane count*/, std::list<std::string>> > laneListTL;
    // list of all 'bike lanes' in each TL
    std::unordered_map< std::string /*TLid*/, std::list<std::string> > bikeLaneListTL;
    // list of all 'side walks' in each TL
    std::unordered_map< std::string /*TLid*/, std::list<std::string> > sideWalkListTL;

    // all incoming lanes in all traffic lights
    std::unordered_map<std::string /*lane*/, std::string /*TLid*/> allIncomingLanes;
    // all outgoing link # for each incoming lane
    std::multimap<std::string /*lane*/, std::pair<std::string /*TLid*/, int /*link number*/>> outgoingLinks;
    // the corresponding lane for each outgoing link #
    std::map<std::pair<std::string /*TLid*/, int /*link*/>, std::string /*lane*/> linkToLane;

private:
    typedef LoopDetectors super;
};

}

#endif
