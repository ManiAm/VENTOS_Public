/****************************************************************************/
/// @file    AID.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
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

#include "nodes/vehicle/03_AID.h"

namespace VENTOS {

Define_Module(VENTOS::ApplV_AID);

ApplV_AID::~ApplV_AID()
{

}


void ApplV_AID::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        AID = par("AID").boolValue();

        fromLane = "";
        toLane = "";
        fromX = 0;
        toX = 0;

        laneChanges.clear();
    }
}


void ApplV_AID::finish()
{
    super::finish();
}


// handle my own SelfMsg
void ApplV_AID::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplV_AID::onBeaconVehicle(BeaconVehicle* wsm)
{
    // no need to pass this down
}


void ApplV_AID::onBeaconRSU(BeaconRSU* wsm)
{
    // no need to pass this down

    // if incident detection is not on, return
    if(!AID)
        return;

    // I received a beacon from RSU
    // send my laneChanges
    if( !laneChanges.empty() )
    {
        LaneChangeMsg* dataMsg = ApplV_AID::prepareData(wsm->getSender(), laneChanges);
        send(dataMsg, lowerLayerOut);
        EV << "### " << SUMOID << ": sent ClaneChangeMsg message." << std::endl;

        laneChanges.clear();
        // lastLaneName = TraCI->commandGetLaneId(SUMOID);
    }
}


LaneChangeMsg*  ApplV_AID::prepareData(std::string receiver, std::deque<std::string> vecValue)
{
    LaneChangeMsg* wsm = new LaneChangeMsg("laneChange", TYPE_LANECHANGE_DATA);

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(dataLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    if(dataOnSCH)
        wsm->setChannelNumber(Veins::Channels::SCH1);
    else
        wsm->setChannelNumber(Veins::Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(dataPriority);
    wsm->setPsid(0);

    wsm->setSender(SUMOID.c_str());
    wsm->setRecipient(receiver.c_str());
    wsm->setLaneChange(vecValue);

    return wsm;
}


// is called, every time the position of vehicle changes
void ApplV_AID::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);

    // if incident detection is not on, return
    if(!AID)
        return;

    toLane = TraCI->vehicleGetLaneID(SUMOID);

    // if we change lane
    if(fromLane != toLane)
    {
        std::ostringstream str;
        toX =  ( TraCI->vehicleGetPosition(SUMOID) ).x;
        str << fromLane <<  "#" << toLane << "#" << fromX << "#" << toX << "#" << omnetpp::simTime().dbl();
        laneChanges.push_back( str.str() );

        fromLane = toLane;
        fromX = toX;
    }
}

}
