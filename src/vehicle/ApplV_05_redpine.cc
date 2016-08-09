/****************************************************************************/
/// @file    ApplV_redpine.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jun 2016
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

#include "ApplV_05_redpine.h"
#include "ExbCIConstants.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVRedpine);

ApplVRedpine::~ApplVRedpine()
{

}


void ApplVRedpine::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        isHIL = par("isHIL").boolValue();
        hardBreakDetection = par("hardBreakDetection").boolValue();
        EEBL = par("EEBL").boolValue();
    }
}


void ApplVRedpine::finish()
{
    super::finish();
}


void ApplVRedpine::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


// is called, every time the position of vehicle changes
void ApplVRedpine::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);

    // if this is an emulated vehicle and hardBreakDetection is active
    if(isHIL && hardBreakDetection)
        checkForHardBreak();
}


// data from the corresponding HIL board
void ApplVRedpine::receiveDataFromBoard(dataEntry* data)
{
    Enter_Method("");

    // todo: react based on the type of data

    // deallocate memory
    delete data;

    // the button is pressed on the HIL board, so this
    // vehicle should stop immediately
    TraCI->vehicleSetSpeed(SUMOID, 0);

    if(EEBL)
    {
        // prepare a 'basic safety message'
        BSM* wsm = new BSM("BSM");
        wsm->setSender(SUMOID.c_str());
        wsm->setSenderType(SUMOType.c_str());
        Coord cord = TraCI->vehicleGetPosition(SUMOID);
        wsm->setPos(cord);

        // and broadcast it to nearby vehicles
        send(wsm, lowerLayerOut);
    }
}


void ApplVRedpine::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    super::onBeaconVehicle(wsm);
}


void ApplVRedpine::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    super::onBeaconRSU(wsm);
}


void ApplVRedpine::onBSM(BSM* wsm)
{
    // check the received data and act on it.
    // all wsm messages here are EEBL for now.
    // the vehicle stops as soon as it receives a EEBL msg.
    // todo: we should decode the received msg to get the msg type

    // if msg is coming from my leading vehicle
    std::vector<std::string> leaderv = TraCI->vehicleGetLeader(SUMOID, sonarDist);
    std::string leader = leaderv[0];
    if(leader == wsm->getSender())
    {
        // signal the reception of EEBL to the corresponding board.
        // This LED notifies the driver about the hard break
        unsigned char data[1] = {EEBL_RECV};
        ExbCI->sendDataToBoard(SUMOID, data, 1);

        // stop immediately
        TraCI->vehicleSetMaxDecel(SUMOID, 4); // increase max decel from default 3 to 4
        TraCI->vehicleSetSpeed(SUMOID, 0);
    }
}


void ApplVRedpine::checkForHardBreak()
{
    static bool breakStarts = false;

    double accel = TraCI->vehicleGetCurrentAccel(SUMOID);

    // as soon as the vehicle breaks hard
    if(!breakStarts && accel <= -3)  // todo: how to check if veh breaks hard?
    {
        // signal 'break start' to the corresponding board
        unsigned char data[1] = {HARD_BREAK_START};
        ExbCI->sendDataToBoard(SUMOID, data, 1);

        breakStarts = true;
    }

    // end of breaking -- vehicle stops completely
    if(breakStarts && accel == 0)
    {
        // signal 'break end' to the corresponding board
        unsigned char data[1] = {HARD_BREAK_END};
        ExbCI->sendDataToBoard(SUMOID, data, 1);

        breakStarts = false;
    }
}

}
