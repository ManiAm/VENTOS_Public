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
#include <Statistics.h>
#include "SignalObj.h"

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

        // get the ptr of the Statistics module
        cModule *module = simulation.getSystemModule()->getSubmodule("statistics");
        Statistics *StatPtr = static_cast<Statistics *>(module);
        if(StatPtr == NULL)
            error("can not get a pointer to the Statistics module.");

        reportBeaconsData = StatPtr->par("reportBeaconsData").boolValue();

        BeaconVehCount = 0;
        BeaconVehDropped = 0;
        BeaconRSUCount = 0;
        PlatoonCount = 0;

        // set parameters in SUMO
        TraCI->vehicleSetDebug(SUMOID, SUMOvehicleDebug);
        TraCI->vehicleSetDowngradeToACC(SUMOID, degradeToACC);

        if(measurementError)
        {
            TraCI->vehicleSetErrorGap(SUMOID, errorGap);
            TraCI->vehicleSetErrorRelSpeed(SUMOID, errorRelSpeed);
        }
        else
        {
            TraCI->vehicleSetErrorGap(SUMOID, 0.);
            TraCI->vehicleSetErrorRelSpeed(SUMOID, 0.);
        }

        // turn off 'strategic' and 'speed gain' lane change in all TL (default is 10 01 01 01 01)
        // todo: is this necessary? lane change causes fault detection in LD
        if(TLControlMode != TL_Router && TLControlMode != TL_OFF)
        {
            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(00, 01, 00, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);   // alter 'lane change' mode
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

    if (std::string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        BeaconVehCount++;

        if( plr == 0 || !dropBeacon(droppT, droppV, plr) )
        {
            ApplVManager::onBeaconVehicle(wsm);

            // report reception to statistics
            if(reportBeaconsData)
            {
                data *pair = new data(wsm->getSender(), SUMOID, 0);
                simsignal_t Signal_beacon = registerSignal("beacon");
                nodePtr->emit(Signal_beacon, pair);
            }
        }
        // drop the beacon, and report it to statistics
        else
        {
            BeaconVehDropped++;

            // report drop to statistics
            if(reportBeaconsData)
            {
                data *pair = new data(wsm->getSender(), SUMOID, 1);
                simsignal_t Signal_beacon = registerSignal("beacon");
                nodePtr->emit(Signal_beacon, pair);
            }
        }
    }
    else if (std::string(wsm->getName()) == "beaconPedestrian")
    {
        BeaconPedestrian* wsm = dynamic_cast<BeaconPedestrian*>(msg);
        ASSERT(wsm);

        BeaconPedCount++;

        ApplVManager::onBeaconPedestrian(wsm);
    }
    else if (std::string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        BeaconRSUCount++;

        ApplVManager::onBeaconRSU(wsm);
    }
    else if(std::string(wsm->getName()) == "platoonMsg")
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        PlatoonCount++;

        ApplVManager::onData(wsm);
    }

    delete msg;
}


// simulate packet loss in application layer
bool ApplVManager::dropBeacon(double time, std::string vehicle, double plr)
{
    if(simTime().dbl() >= time)
    {
        // vehicle == "" --> drop beacon in all vehicles
        // vehicle == SUMOID --> drop beacon only in specified vehicle
        if (vehicle == "" || vehicle == SUMOID)
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


void ApplVManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    ApplVCoordinator::onBeaconVehicle(wsm);

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
            TraCI->vehicleSetControllerParameters(SUMOID, buffer);
        }
    }
    // model is CACC with platoon leader communication (TypeCACC2)
    else if(SUMOControllerType == SUMO_TAG_CF_CACC && SUMOControllerNumber == 2)
    {
        if(plnMode == platoonOff)
            error("no platoon leader is present! check plnMode!");

        // I am platoon leader
        // get data from my leading vehicle
        if(myPlnDepth == 0)
        {
            if( isBeaconFromLeading(wsm) )
            {
                char buffer [200];
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "preceding");
                TraCI->vehicleSetControllerParameters(SUMOID, buffer);
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
                TraCI->vehicleSetControllerParameters(SUMOID, buffer);
            }
        }
    }
    else
    {
        error("not a valid control type or control number!");
    }
}


void ApplVManager::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    // pass it down
    // ApplVCoordinator::onBeaconPedestrian(wsm);

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

