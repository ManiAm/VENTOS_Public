/****************************************************************************/
/// @file    ApplV_08_Manager.cc
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

#include "ApplV_08_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVManager);

ApplVManager::~ApplVManager()
{

}


void ApplVManager::initialize(int stage)
{
    ApplVCoordinator::initialize(stage);

	if (stage == 0)
	{
        // NED variables
        degradeToACC = par("degradeToACC").boolValue();
        SUMOvehicleDebug = par("SUMOvehicleDebug").boolValue();

        // NED variables (packet loss ratio)
        droppT = par("droppT").doubleValue();
        droppV = par("droppV").stringValue();
        plr = par("plr").doubleValue();

        // NED variables (measurement errors)
        measurementError = par("measurementError").boolValue();
        errorGap = par("errorGap").doubleValue();
        errorRelSpeed = par("errorRelSpeed").doubleValue();

        BeaconVehCount = 0;
        BeaconVehDropped = 0;
        BeaconRSUCount = 0;
        PlatoonCount = 0;

        WATCH(BeaconVehCount);
        WATCH(BeaconVehDropped);
        WATCH(BeaconRSUCount);
        WATCH(PlatoonCount);

        // set parameters in SUMO
        TraCI->vehicleSetDebug(SUMOvID, SUMOvehicleDebug);
        TraCI->vehicleSetDowngradeToACC(SUMOvID, degradeToACC);

        if(measurementError)
        {
            TraCI->vehicleSetErrorGap(SUMOvID, errorGap);
            TraCI->vehicleSetErrorRelSpeed(SUMOvID, errorRelSpeed);
        }
        else
        {
            TraCI->vehicleSetErrorGap(SUMOvID, 0.);
            TraCI->vehicleSetErrorRelSpeed(SUMOvID, 0.);
        }
	}
}


void ApplVManager::finish()
{
    ApplVCoordinator::finish();
}


void ApplVManager::handleSelfMsg(cMessage* msg)
{
    ApplVCoordinator::handleSelfMsg(msg);
}


void ApplVManager::handleLowerMsg(cMessage* msg)
{
    // vehicles other than CACC should ignore the received msg
    if( !VANETenabled )
    {
        delete msg;
        return;
    }

    // check if jamming attack is effective?
    // todo: change this later
    cModule *module = simulation.getSystemModule()->getSubmodule("adversary", 0);

    if(module != NULL)
    {
        cModule *applModule = module->getSubmodule("appl");

        bool jammingActive = applModule->par("jammingAttack").boolValue();
        double AttackT = applModule->par("AttackT").doubleValue();

        if(jammingActive && simTime().dbl() > AttackT)
        {
            delete msg;
            return;
        }
    }

    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        BeaconVehCount++;

        if( !dropBeacon(droppT, droppV, plr) )
        {
            ApplVManager::onBeaconVehicle(wsm);
        }
        // drop the beacon, and report it to statistics
        else
        {
            BeaconVehDropped++;
            reportDropToStatistics(wsm);
        }
    }
    if (string(wsm->getName()) == "beaconPedestrian")
    {
        BeaconPedestrian* wsm = dynamic_cast<BeaconPedestrian*>(msg);
        ASSERT(wsm);

        BeaconPedCount++;

        ApplVManager::onBeaconPedestrian(wsm);
    }
    else if (string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        BeaconRSUCount++;

        ApplVManager::onBeaconRSU(wsm);
    }
    else if(string(wsm->getName()) == "platoonMsg")
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        PlatoonCount++;

        ApplVManager::onData(wsm);
    }

    delete msg;
}


// simulate packet loss in application layer
bool ApplVManager::dropBeacon(double time, string vehicle, double plr)
{
    if(simTime().dbl() >= time)
    {
        // vehicle == "" --> drop beacon in all vehicles
        // vehicle == SUMOvID --> drop beacon only in specified vehicle
        if (vehicle == "" || vehicle == SUMOvID)
        {
            // random number in [0,1)
            double p = dblrand();

            if( p < (plr/100) )
                return true;   // drop the beacon
            else
                return false;  // keep the beacon
        }
        else
            return false;
    }
    else
        return false;
}


void ApplVManager::reportDropToStatistics(BeaconVehicle* wsm)
{
    // todo:
    if(true /*one_vehicle_look_ahead*/)
    {
        bool result =  isBeaconFromLeading(wsm);

        if(result)
        {
            // a beacon from preceding vehicle is drooped
            simsignal_t Signal_beaconD = registerSignal("beaconD");
            nodePtr->emit(Signal_beaconD, 1);
        }
        else
        {
            // a beacon from other vehicle is drooped
            simsignal_t Signal_beaconD = registerSignal("beaconD");
            nodePtr->emit(Signal_beaconD, 2);
        }
    }
    else
    {
        bool result = isBeaconFromMyPlatoonLeader(wsm);

        // todo:
        if(result)
        {
            // a beacon from platoon leader is drooped
            simsignal_t Signal_beaconD = registerSignal("beaconD");
            nodePtr->emit(Signal_beaconD, 3);
        }
        else
        {
            // a beacon from other vehicle is drooped
            simsignal_t Signal_beaconD = registerSignal("beaconD");
            nodePtr->emit(Signal_beaconD, 2);
        }
    }
}


void ApplVManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    ApplVCoordinator::onBeaconVehicle(wsm);

    EV << "## " << SUMOvID << " received beacon ..." << endl;
    ApplVBeacon::printBeaconContent(wsm);

    // model is Krauss (TypeManual)
    if(SUMOControllerType == SUMO_TAG_CF_KRAUSS)
    {
        // we ignore all received BeaconVehicles
    }
    // model is ACC with controllerNumber = 1 (TypeACC1)
    else if(SUMOControllerType == SUMO_TAG_CF_ACC && SUMOControllerNumber == 1)
    {
        // we ignore all received BeaconVehicles
    }
    // model is CACC with one-vehicle look-ahead communication (TypeCACC1)
    else if(SUMOControllerType == SUMO_TAG_CF_CACC && SUMOControllerNumber == 1)
    {
        if( isBeaconFromLeading(wsm) )
        {
            char buffer [200];
            sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "preceding");
            TraCI->vehicleSetControllerParameters(SUMOvID, buffer);
        }
    }
    // model is CACC with platoon leader communication (TypeCACC2)
    else if(SUMOControllerType == SUMO_TAG_CF_CACC && SUMOControllerNumber == 2)
    {
        if(plnMode == 1)
            error("no platoon leader is present! check plnMode!");

        // I am platoon leader
        // get data from my leading vehicle
        if(myPlnDepth == 0)
        {
            if( isBeaconFromLeading(wsm) )
            {
                char buffer [200];
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "preceding");
                TraCI->vehicleSetControllerParameters(SUMOvID, buffer);
            }
        }
        // I am a follower
        // get data from my platoon leader
        else
        {
            if( isBeaconFromMyPlatoonLeader(wsm) )
            {
                char buffer [200];
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "leader");
                TraCI->vehicleSetControllerParameters(SUMOvID, buffer);
            }
        }
    }
    else
    {
        error("not a valid control type or control number!");
    }


//    // send results to SUMO if result = true
//    if(result)
//    {
//        char buffer [200];
//        sprintf (buffer, "%f#%f#%f#%f#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender() );
//        TraCI->commandSetCFParameters(SUMOvID, buffer);
//
//        // a beacon from the leading vehicle or platoon leader is received
//        data *pair = new data(wsm->getSender());
//        simsignal_t Signal_beaconP = registerSignal("beaconP");
//        nodePtr->emit(Signal_beaconP, pair);
//    }
//    else
//    {
//        // a beacon from other vehicles is received
//        data *pair = new data(wsm->getSender());
//        simsignal_t Signal_beaconO = registerSignal("beaconO");
//        nodePtr->emit(Signal_beaconO, pair);
//    }

}


void ApplVManager::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    // pass it down
    // ApplVCoordinator::onBeaconPedestrian(wsm);

    EV << "## " << SUMOvID << " received beacon ..." << endl;
    //ApplVBeacon::printBeaconContent(wsm);

    char buffer [200];
    sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "preceding");
}


void ApplVManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    ApplVCoordinator::onBeaconRSU(wsm);
}


void ApplVManager::onData(PlatoonMsg* wsm)
{
    // pass it down
    ApplVCoordinator::onData(wsm);
}


// is called, every time the position of vehicle changes
void ApplVManager::handlePositionUpdate(cObject* obj)
{
    ApplVCoordinator::handlePositionUpdate(obj);
}


}

