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

#include "Warmup.h"

namespace VENTOS {

Define_Module(VENTOS::Warmup);


Warmup::~Warmup()
{

}


void Warmup::initialize(int stage)
{
    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to the VehicleSpeedProfile module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("speedprofile");
        ASSERT(module);
        SpeedProfilePtr = static_cast<SpeedProfile *>(module);

        // get totoal vehicles from addMobileNode module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addMobileNode");
        ASSERT(module);
        numVehicles = module->par("numVehicles").longValue();

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);

        active = par("active").boolValue();
        laneId = par("laneId").stringValue();
        stopPosition = par("stopPosition").doubleValue() * numVehicles;
        warmUpSpeed = par("warmUpSpeed").doubleValue();
        waitingTime = par("waitingTime").doubleValue();

        startTime = -1;
        IsWarmUpFinished = false;

        if(active)
            finishingWarmup = new omnetpp::cMessage("finishingWarmup", 1);
    }
}


void Warmup::finish()
{
    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void Warmup::handleMessage(omnetpp::cMessage *msg)
{
    if (msg == finishingWarmup)
    {
        IsWarmUpFinished = true;
        std::cout << "t=" << omnetpp::simTime().dbl() << ": Warm-up phase finished." << std::endl;
    }
}


void Warmup::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        // if warm-up is active and not finished
        if(active && !IsWarmUpFinished)
        {
            if(!finishingWarmup->isScheduled())
            {
                // perform warm-up for this time step
                bool finished = Warmup::DoWarmup();

                // if warm-up is finished, start speed-profiling
                if (finished)
                    SpeedProfilePtr->Change();
            }
        }
        // do speed-profiling
        else
            SpeedProfilePtr->Change();
    }
}


bool Warmup::DoWarmup()
{
    // who is leading?
    std::list<std::string> veh = TraCI->laneGetLastStepVehicleIDs(laneId);

    if(veh.empty())
        return false;

    // store the current simulation time as startTime
    if(startTime == -1)
    {
        startTime = omnetpp::simTime().dbl();
        std::cout << std::endl;
        std::cout << "t=" << omnetpp::simTime().dbl() << ": Warm-up phase is started ..." << std::endl;
    }

    // get the first leading vehicle
    std::string leadingVehicle = veh.back();

    double pos = TraCI->vehicleGetLanePosition(leadingVehicle);

    // we are at stop position
    if(pos >= stopPosition)
    {
        // set speed to warmupSpeed and wait for other vehicles
        TraCI->vehicleSetSpeed(leadingVehicle, warmUpSpeed);

        // get # of vehicles that have entered simulation so far
        int n = TraCI->vehicleGetIDCount();

        // if all vehicles are in the simulation, then wait for waitingTime before finishing warm-up
        if(n == numVehicles)
        {
            scheduleAt(omnetpp::simTime() + waitingTime, finishingWarmup);
            std::cout << "t=" << omnetpp::simTime().dbl() << ": Waiting for " << waitingTime << "s before finishing warm-up ..." << std::endl;
        }
    }

    return false;
}

}

