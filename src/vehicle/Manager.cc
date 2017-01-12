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

#include "vehicle/Manager.h"
#include "global/SignalObj.h"

namespace VENTOS {

std::vector<ApplVManager::BeaconStat_t> ApplVManager::Vec_Beacons;

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

        // set parameters in SUMO
        TraCI->vehicleSetDebug(SUMOID, par("SUMOvehicleDebug").boolValue());
        TraCI->vehicleSetDowngradeToACC(SUMOID, par("degradeToACC").boolValue());

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
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TrafficLight");
        int TLControlMode = module->par("TLControlMode").longValue();
        if(TLControlMode != TL_Router && TLControlMode != TL_OFF)
        {
            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(00, 01, 00, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);   // alter 'lane change' mode
        }
    }
}


void ApplVManager::finish()
{
    super::finish();

    if(record_beacon_stat)
        save_beacon_stat_toFile();
}


void ApplVManager::receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signalID, omnetpp::cObject* obj, cObject* details)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
    // pass it up, if we do not know how to handle the signal
    else
        super::receiveSignal(source, signalID, obj, details);
}


void ApplVManager::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplVManager::handleLowerMsg(omnetpp::cMessage* msg)
{
    // Only DSRC-enabled vehicles accept this msg
    if( !DSRCenabled )
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
                Vec_Beacons.push_back(entry);
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
                Vec_Beacons.push_back(entry);
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

    // model is Krauss (TypeManual)
    if(SUMOControllerType == SUMO_TAG_CF_KRAUSS || SUMOControllerType == SUMO_TAG_CF_KRAUSS_ORIG1)
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
            sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (omnetpp::simTime().dbl())*1000, wsm->getSender(), "preceding");
            TraCI->vehicleSetControllerParameters(SUMOID, buffer);
        }
    }
    // model is CACC with platoon leader communication (TypeCACC2)
    else if(SUMOControllerType == SUMO_TAG_CF_CACC && SUMOControllerNumber == 2)
    {
        if(plnMode == platoonOff)
            throw omnetpp::cRuntimeError("no platoon leader is present! check plnMode!");

        // I am platoon leader
        // get data from my leading vehicle
        if(myPlnDepth == 0)
        {
            if( isBeaconFromLeading(wsm) )
            {
                char buffer [200];
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (omnetpp::simTime().dbl())*1000, wsm->getSender(), "preceding");
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
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (omnetpp::simTime().dbl())*1000, wsm->getSender(), "leader");
                TraCI->vehicleSetControllerParameters(SUMOID, buffer);
            }
        }
    }
    else
    {
        throw omnetpp::cRuntimeError("not a valid control type or control number!");
    }
}


void ApplVManager::onBeaconBicycle(BeaconBicycle* wsm)
{
    // pass it down
    super::onBeaconBicycle(wsm);
}


void ApplVManager::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    // pass it down
    super::onBeaconPedestrian(wsm);

    char buffer [200];
    sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (omnetpp::simTime().dbl())*1000, wsm->getSender(), "preceding");
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


void ApplVManager::save_beacon_stat_toFile()
{
    if(Vec_Beacons.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_beaconsStat.txt") % currentRun;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file in CMD mode
    if(!omnetpp::cSimulation::getActiveEnvir()->isGUI())
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from");
    fprintf (filePtr, "%-20s","to");
    fprintf (filePtr, "%-20s\n\n","dropped");

    for(auto &y : Vec_Beacons)
    {
        fprintf (filePtr, "%-12.2f", y.time);
        fprintf (filePtr, "%-20s", y.senderID.c_str());
        fprintf (filePtr, "%-20s", y.receiverID.c_str());
        fprintf (filePtr, "%-20d \n", y.dropped);
    }

    fclose(filePtr);
}

}
