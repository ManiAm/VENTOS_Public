/****************************************************************************/
/// @file    ApplV_OBU.cc
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

#include <ApplV_05_OBU.h>
#include "ExbCIConstants.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVOBU);

ApplVOBU::~ApplVOBU()
{

}


void ApplVOBU::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        hasOBU = par("hasOBU").boolValue();
        forwardCollisionDetection = par("forwardCollisionDetection").boolValue();
        EEBL = par("EEBL").boolValue();

        // all emulated vehicles are subscribed to mobilityStateChangedSignal in
        // order to detect forward collision
        if(hasOBU && forwardCollisionDetection)
        {
            findHost()->subscribe(mobilityStateChangedSignal, this);
            LOG_INFO << boost::format("\n>>> %1% is subscribed to mobilityStateChangedSignal. \n") % SUMOID << std::flush;
        }
    }
}


void ApplVOBU::finish()
{
    super::finish();

    if(hasOBU && forwardCollisionDetection)
        findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplVOBU::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


// is called, every time the position of vehicle changes
void ApplVOBU::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);

    // if this is an emulated vehicle and forwardCollisionDetection is active
    if(hasOBU && forwardCollisionDetection)
        checkForwardCollision();
}


// data from the corresponding OBU
void ApplVOBU::receiveDataFromOBU(dataEntry* data)
{
    Enter_Method("");

    // 'emergency break' signal is received from the OBU
    if(data->bufferLength == 1 && *(data->buffer) == Signal_EmergencyBreak)
    {
        LOG_INFO << boost::format("\n%1%: 'Emergency Break' signal received from OBU. \n") % SUMOID << std::flush;

        // stop immediately
        TraCI->vehicleSetMaxDecel(SUMOID, 7);
        TraCI->vehicleSetSpeed(SUMOID, 0);
    }
    // WSM message is received from the OBU
    else if(*(data->buffer) == 0x34)  // todo: how to detect a WSM message?
    {
        if(EEBL)
        {
            // prepare a 'Basic Safety Message (BSM)'
            BSM* wsm = new BSM("BSM");
            wsm->setSender(SUMOID.c_str());
            wsm->setSenderType(SUMOType.c_str());
            Coord cord = TraCI->vehicleGetPosition(SUMOID);
            wsm->setPos(cord);

            // and broadcast it to nearby vehicles
            send(wsm, lowerLayerOut);
        }
    }
    // todo
    //else
    //    throw omnetpp::cRuntimeError("Unknown data of size %d from OBU %s", data->bufferLength, data->ipv4.c_str());

    // deallocate memory
    delete data;
}


void ApplVOBU::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    super::onBeaconVehicle(wsm);
}


void ApplVOBU::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    super::onBeaconRSU(wsm);
}


void ApplVOBU::onBSM(BSM* wsm)
{
    // check the received data and act on it.
    // all wsm messages here are EEBL for now.
    // the vehicle stops as soon as it receives a EEBL msg.
    // todo: we should decode the received msg to get the msg type

    // get the leading vehicle
    std::vector<std::string> leaderv = TraCI->vehicleGetLeader(SUMOID, sonarDist);
    std::string leader = leaderv[0];

    // if EEBL msg is coming from my leading vehicle
    if(leader == wsm->getSender())
    {
        // if I have an OBU
        if(hasOBU)
        {
            // send EEBL signal to my OBU
            unsigned char data[1] = {Signal_EEBL};
            ExbCI->sendDataToBoard(SUMOID, data, 1);
        }
    }
}


void ApplVOBU::checkForwardCollision()
{
    // get current speed
    double v = TraCI->vehicleGetSpeed(SUMOID);

    // get maximum deceleration (only once)
    static double max_decel = -1;
    if(max_decel == -1)
        max_decel = TraCI->vehicleGetMaxDecel(SUMOID);

    // get the leading vehicle + distance to leading vehicle
    std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOID, sonarDist);
    std::string vleaderID = vleaderIDnew[0];
    double distance = std::stof(vleaderIDnew[1]);

    // get speed of the front vehicle
    double v_f = TraCI->vehicleGetSpeed(vleaderID);

    // get maximum deceleration of front vehicle (only once)
    static double max_decel_f = -1;
    if(max_decel_f == -1)
        max_decel_f = TraCI->vehicleGetMaxDecel(vleaderID);

    // compute the breaking distance
    double breakingDist = ( (v * v) / (2 * max_decel) ) - ( (v_f * v_f) / (2 * max_decel_f) );

    static bool forwardCollisionDetected = false;

    if(distance < breakingDist)
    {
        if(!forwardCollisionDetected)
        {
            LOG_INFO << boost::format("\n%1%: 'Forward Collision Warning' signal sent to OBU. \n") % SUMOID << std::flush;

            // send ForwardCollisionWarning signal to the OBU
            unsigned char data[1] = {Signal_ForwardCollisionWarning};
            ExbCI->sendDataToBoard(SUMOID, data, 1);

            forwardCollisionDetected = true;
        }
    }
    // reset forwardCollisionDetected as soon as distance >= breakingDist
    else
        forwardCollisionDetected = false;
}

}
