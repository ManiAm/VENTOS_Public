/****************************************************************************/
/// @file    VehicleWarmup.cc
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

#include "VehicleWarmup.h"

namespace VENTOS {

Define_Module(VENTOS::Warmup);


Warmup::~Warmup()
{

}


void Warmup::initialize(int stage)
{
    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        // get a pointer to the VehicleSpeedProfile module
        module = simulation.getSystemModule()->getSubmodule("speedprofile");
        SpeedProfilePtr = static_cast<SpeedProfile *>(module);

        // get totoal vehicles from AddVehicle module
        totalVehicles = simulation.getSystemModule()->getSubmodule("addVehicle")->par("totalVehicles").longValue();

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        on = par("on").boolValue();
        laneId = par("laneId").stringValue();
        stopPosition = totalVehicles * par("stopPosition").doubleValue();
        warmUpSpeed = par("warmUpSpeed").doubleValue();
        waitingTime = par("waitingTime").doubleValue();

        startTime = -1;
        IsWarmUpFinished = false;

        if(on)
        {
            warmupFinish = new cMessage("warmupFinish", 1);
        }
    }
}


void Warmup::finish()
{


}


void Warmup::handleMessage(cMessage *msg)
{
    if (msg == warmupFinish)
    {
        IsWarmUpFinished = true;
    }
}


void Warmup::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        // check if warm-up phase is finished
        bool finished = Warmup::DoWarmup();
        if (finished)
        {
            // we can start speed profiling
            SpeedProfilePtr->Change();
        }
    }
}


bool Warmup::DoWarmup()
{
    // if warmup is not on, return finished == true
    if (!on)
        return true;

    if(IsWarmUpFinished)
        return true;

    if( warmupFinish->isScheduled() )
        return false;

    // who is leading?
    std::list<std::string> veh = TraCI->laneGetLastStepVehicleIDs(laneId);

    if(veh.empty())
        return false;

    // if startTime is not specified, then store the current simulation time as startTime
    if(startTime == -1)
    {
        startTime = simTime().dbl();
    }
    // if user specifies a startTime, but it is negative
    else if(startTime < 0)
    {
        error("startTime is less than 0 in Warmup.");
    }
    // if user specifies a startTime, but we should wait for it
    else if(startTime > simTime().dbl())
        return false;

    // get the first leading vehicle
    std::string leadingVehicle = veh.back();

    double pos = TraCI->vehicleGetLanePosition(leadingVehicle);

    // we are at stop position
    if(pos >= stopPosition)
    {
        // start breaking and wait for other vehicles
        TraCI->vehicleSetSpeed(leadingVehicle, warmUpSpeed);

        // get # of vehicles that have entered simulation so far
        int n = TraCI->vehicleGetIDCount();

        // if all vehicles are in the simulation
        if( n == totalVehicles )
        {
            scheduleAt(simTime() + waitingTime, warmupFinish);
        }
    }

    return false;
}

}

