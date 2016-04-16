/****************************************************************************/
/// @file    TrafficLights.cc
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

#include <03_TrafficLights.h>
#include <iomanip>

namespace VENTOS {

Define_Module(VENTOS::TrafficLights);


TrafficLights::~TrafficLights()
{

}


void TrafficLights::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        laneListTL.clear();
        bikeLaneListTL.clear();
        sideWalkListTL.clear();

        allIncomingLanes.clear();
        outgoingLinks.clear();
        linkToLane.clear();
    }
}


void TrafficLights::finish()
{
    super::finish();
}


void TrafficLights::handleMessage(cMessage *msg)
{
    super::handleMessage(msg);
}


void TrafficLights::initialize_withTraCI()
{
    super::initialize_withTraCI();

    TLList = TraCI->TLGetIDList();

    // for each traffic light
    for (auto &it : TLList)
    {
        std::string TLid = it;

        // get all incoming lanes
        std::list<std::string> lan = TraCI->TLGetControlledLanes(it);

        // remove duplicate entries
        lan.unique();

        laneListTL[TLid] = std::make_pair(lan.size(),lan);

        std::list<std::string> bikeLaneList;
        std::list<std::string> sideWalkList;

        // for each incoming lane
        for(auto &it2 : lan)
        {
            std::string lane = it2;

            allIncomingLanes[lane] = TLid;

            // store all bike lanes and side walks
            std::list<std::string> allowedClasses = TraCI->laneGetAllowedClasses(lane);
            if(allowedClasses.size() == 1 && allowedClasses.front() == "bicycle")
                bikeLaneList.push_back(lane);
            else if(allowedClasses.size() == 1 && allowedClasses.front() == "pedestrian")
                sideWalkList.push_back(lane);
        }

        bikeLaneListTL[TLid] = bikeLaneList;
        sideWalkListTL[TLid] = sideWalkList;
        bikeLaneList.clear();
        sideWalkList.clear();

        // get all links controlled by this TL
        std::map<int,std::vector<std::string>> result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(auto &it2 : result)
        {
            int linkNumber = it2.first;
            std::vector<std::string> link = it2.second;
            std::string incommingLane = link[0];

            outgoingLinks.insert( std::make_pair(incommingLane, std::make_pair(TLid,linkNumber)) );

            linkToLane.insert( std::make_pair(std::make_pair(TLid,linkNumber), incommingLane) );
        }
    }
}


void TrafficLights::executeEachTimeStep()
{
    super::executeEachTimeStep();

}

}
