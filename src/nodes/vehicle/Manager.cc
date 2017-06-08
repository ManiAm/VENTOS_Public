/****************************************************************************/
/// @file    Manager.cc
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

#include "nodes/vehicle/Manager.h"
#include "global/SignalObj.h"

namespace VENTOS {

#define PARAMS_DELIM  "#"

Define_Module(VENTOS::ApplVManager);

ApplVManager::~ApplVManager()
{

}


void ApplVManager::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        // NED variables (packet loss ratio)
        droppT = par("droppT").doubleValue();
        droppV = par("droppV").stringValue();
        plr = par("plr").doubleValue();

        // NED variables (measurement errors)
        measurementError = par("measurementError").boolValue();
        errorGap = par("errorGap").doubleValue();
        errorRelSpeed = par("errorRelSpeed").doubleValue();

        record_beacon_stat = par("record_beacon_stat").boolValue();

        BeaconVehCount = 0;
        BeaconVehDropped = 0;
        BeaconRSUCount = 0;
        PlatoonCount = 0;

        carFollowingModelId = TraCI->vehicleGetCarFollowingModelID(SUMOID);

        TraCI->vehicleSetDebug(SUMOID, getParentModule()->par("SUMOvehicleDebug").boolValue());

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
    }
}


void ApplVManager::finish()
{
    super::finish();
}


void ApplVManager::receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signalID, omnetpp::cObject* obj, cObject* details)
{
    Enter_Method_Silent();

    super::receiveSignal(source, signalID, obj, details);
}


void ApplVManager::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplVManager::handleLowerMsg(omnetpp::cMessage* msg)
{
    // Only DSRC-enabled vehicles accept this msg
    if(!DSRCenabled)
    {
        delete msg;
        return;
    }

    // check if jamming attack is effective?
    // todo: change this later
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("adversary", 0);
    if(module != NULL)
    {
        cModule *applModule = module->getSubmodule("appl");

        bool jammingActive = applModule->par("jammingAttack").boolValue();
        double AttackT = applModule->par("AttackT").doubleValue();

        if(jammingActive && omnetpp::simTime().dbl() > AttackT)
        {
            delete msg;
            return;
        }
    }

    onMessageType(msg);

    delete msg;
}


// is called, every time the position of vehicle changes
void ApplVManager::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);
}


void ApplVManager::onMessageType(omnetpp::cMessage* msg)
{
    if (msg->getKind() == TYPE_BEACON_VEHICLE)
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        BeaconVehCount++;

        if( plr == 0 || !dropBeacon(droppT, droppV, plr) )
        {
            onBeaconVehicle(wsm);

            // report reception to statistics
            if(record_beacon_stat)
            {
                BeaconStat_t entry = {omnetpp::simTime().dbl(), wsm->getSender() /*sender*/, SUMOID /*receiver*/, 0 /*dropped?*/};
                STAT->global_Beacon_stat.push_back(entry);
            }
        }
        // drop the beacon, and report it to statistics
        else
        {
            BeaconVehDropped++;

            // report drop to statistics
            if(record_beacon_stat)
            {
                BeaconStat_t entry = {omnetpp::simTime().dbl(), wsm->getSender() /*sender*/, SUMOID /*receiver*/, 1 /*dropped?*/};
                STAT->global_Beacon_stat.push_back(entry);
            }
        }
    }
    else if (msg->getKind() == TYPE_BEACON_BICYCLE)
    {
        BeaconBicycle* wsm = dynamic_cast<BeaconBicycle*>(msg);
        ASSERT(wsm);

        BeaconBikeCount++;

        onBeaconBicycle(wsm);
    }
    else if (msg->getKind() == TYPE_BEACON_PEDESTRIAN)
    {
        BeaconPedestrian* wsm = dynamic_cast<BeaconPedestrian*>(msg);
        ASSERT(wsm);

        BeaconPedCount++;

        onBeaconPedestrian(wsm);
    }
    else if (msg->getKind() == TYPE_BEACON_RSU)
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        BeaconRSUCount++;

        onBeaconRSU(wsm);
    }
    else if(msg->getKind() == TYPE_PLATOON_DATA)
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        PlatoonCount++;

        onPlatoonMsg(wsm);
    }
    // todo
    else if(msg->getKind() == TYPE_CRL_PIECE)
    {


    }
    else
        throw omnetpp::cRuntimeError("Vehicle %s received unsupported msg %s of type %d!", SUMOID.c_str(), msg->getName(), msg->getKind());
}


// simulate packet loss in application layer
bool ApplVManager::dropBeacon(double time, std::string vehicle, double plr)
{
    if(omnetpp::simTime().dbl() >= time)
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
    super::onBeaconVehicle(wsm);

    // I am a CACC vehicle
    if(carFollowingModelId == SUMO_CF_CACC)
    {
        // I receive a beacon from a vehicle in my platoon
        if(isBeaconFromMyPlatoon(wsm))
        {
            std::ostringstream params;

            // Note: DO NOT change the order of parameters

            // parameters from the beacon
            params << (omnetpp::simTime().dbl())*1000 << PARAMS_DELIM;
            params << wsm->getSender() << PARAMS_DELIM;
            params << (double)wsm->getSpeed() << PARAMS_DELIM;
            params << (double)wsm->getAccel() << PARAMS_DELIM;
            params << (double)wsm->getMaxDecel() << PARAMS_DELIM;

            // my own parameters
            params << myPlnID << PARAMS_DELIM;
            params << myPlnDepth << PARAMS_DELIM;
            params << plnSize;

            // update my platoon view in SUMO
            TraCI->vehiclePlatoonViewUpdate(SUMOID, params.str());
        }
    }
}


void ApplVManager::onBeaconBicycle(BeaconBicycle* wsm)
{
    // pass it down
    //super::onBeaconBicycle(wsm);
}


void ApplVManager::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    // pass it down
    //super::onBeaconPedestrian(wsm);
}


void ApplVManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    super::onBeaconRSU(wsm);
}


void ApplVManager::onPlatoonMsg(PlatoonMsg* wsm)
{
    // pass it down
    super::onPlatoonMsg(wsm);
}


double ApplVManager::calculateCO2emission(double v, double a)
{
    // Calculate CO2 emission parameters according to:
    // Cappiello, A. and Chabini, I. and Nam, E.K. and Lue, A. and Abou Zeid, M., "A statistical model of vehicle emissions and fuel consumption," IEEE 5th International Conference on Intelligent Transportation Systems (IEEE ITSC), pp. 801-809, 2002

    double A = 1000 * 0.1326; // W/m/s
    double B = 1000 * 2.7384e-03; // W/(m/s)^2
    double C = 1000 * 1.0843e-03; // W/(m/s)^3
    double M = 1325.0; // kg

    // power in W
    double P_tract = A*v + B*v*v + C*v*v*v + M*a*v; // for sloped roads: +M*g*sin_theta*v

    /*
    // "Category 7 vehicle" (e.g. a '92 Suzuki Swift)
    double alpha = 1.01;
    double beta = 0.0162;
    double delta = 1.90e-06;
    double zeta = 0.252;
    double alpha1 = 0.985;
     */

    // "Category 9 vehicle" (e.g. a '94 Dodge Spirit)
    double alpha = 1.11;
    double beta = 0.0134;
    double delta = 1.98e-06;
    double zeta = 0.241;
    double alpha1 = 0.973;

    if (P_tract <= 0)
        return alpha1;

    return alpha + beta*v*3.6 + delta*v*v*v*(3.6*3.6*3.6) + zeta*a*v;
}

}
