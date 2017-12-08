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
        dropStartTime = par("dropStartTime").doubleValue();
        plr = par("plr").doubleValue();
        if(plr < 0 || plr > 100)
            throw omnetpp::cRuntimeError("Packet Loss Ratio (PLR) should be in range [0,100]: %d", plr);

        record_beacon_stat = par("record_beacon_stat").boolValue();

        carFollowingModelId = TraCI->vehicleGetCarFollowingModelID(SUMOID);

        if(par("measurementError").boolValue())
        {
            TraCI->vehicleSetErrorGap(SUMOID, par("errorGap").doubleValue());
            TraCI->vehicleSetErrorRelSpeed(SUMOID, par("errorRelSpeed").doubleValue());
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

    // jamming attack is going on
    if(jamming)
    {
        delete msg;
        return;
    }

    onMessageType(msg);
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

        if( plr == 0 || !dropBeacon(dropStartTime, plr) )
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

        delete msg;
    }
    else if (msg->getKind() == TYPE_BEACON_BICYCLE)
    {
        BeaconBicycle* wsm = dynamic_cast<BeaconBicycle*>(msg);
        ASSERT(wsm);

        BeaconBikeCount++;

        //onBeaconBicycle(wsm);

        delete msg;
    }
    else if (msg->getKind() == TYPE_BEACON_PEDESTRIAN)
    {
        BeaconPedestrian* wsm = dynamic_cast<BeaconPedestrian*>(msg);
        ASSERT(wsm);

        BeaconPedCount++;

        //onBeaconPedestrian(wsm);

        delete msg;
    }
    else if (msg->getKind() == TYPE_BEACON_RSU)
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        BeaconRSUCount++;

        onBeaconRSU(wsm);

        delete msg;
    }
    else if(msg->getKind() == TYPE_PLATOON_DATA)
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        PlatoonCount++;

        onPlatoonMsg(wsm);

        delete msg;
    }
    else if(msg->getKind() == TYPE_GENERIC_DATA)
    {
        dataMsg* wsm = dynamic_cast<dataMsg*>(msg);
        ASSERT(wsm);

        onDataMsg(wsm);
    }
    // todo
    else if(msg->getKind() == TYPE_CRL_PIECE)
    {
        delete msg;
    }
    else
        throw omnetpp::cRuntimeError("Vehicle %s received unsupported msg %s of type %d!", SUMOID.c_str(), msg->getName(), msg->getKind());
}


// simulate packet loss in application layer
bool ApplVManager::dropBeacon(double time, double plr)
{
    if(omnetpp::simTime().dbl() >= time)
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
            params << wsm->getPlatoonDepth() << PARAMS_DELIM;
            params << (double)wsm->getSpeed() << PARAMS_DELIM;
            params << (double)wsm->getAccel() << PARAMS_DELIM;
            params << (double)wsm->getMaxDecel() << PARAMS_DELIM;

            // my own parameters
            params << myPlnID << PARAMS_DELIM;
            params << myPlnDepth;

            // update my platoon view in SUMO
            TraCI->vehiclePlatoonViewUpdate(SUMOID, params.str());
        }
    }
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
