/****************************************************************************/
/// @file    ApplRSU_03_TL_VANET.cc
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

#include "ApplRSU_03_TL_VANET.h"

namespace VENTOS {

std::vector<detectedVehicleEntry> ApplRSUTLVANET::Vec_detectedVehicles;

Define_Module(VENTOS::ApplRSUTLVANET);

ApplRSUTLVANET::~ApplRSUTLVANET()
{

}


void ApplRSUTLVANET::initialize(int stage)
{
    ApplRSUAID::initialize(stage);

    // get a pointer to the TrafficLight module
    cModule *module = simulation.getSystemModule()->getSubmodule("TrafficLight");
    TLControlMode = module->par("TLControlMode").longValue();
    minGreenTime = module->par("minGreenTime").doubleValue();

    if (TLControlMode != TL_MultiClass)
        return;

    if (stage==0)
    {
        // for the TL_VANET we need this RSU to be associated with a TL
        if(myTLid == "")
            error("The id of %s does not match with any TL. Check RSUsLocation.xml file!", myFullId);

        collectVehApproach = par("collectVehApproach").boolValue();

        Vec_detectedVehicles.clear();

        setDetectionRegion();

        // for each incoming lane in this TL
        std::list<std::string> lan = TraCI->TLGetControlledLanes(myTLid);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
        {
            std::string lane = *it2;

            lanesTL[lane] = myTLid;

            // get the max speed on this lane
            double maxV = TraCI->laneGetMaxSpeed(lane);

            // calculate initial passageTime for this lane
            // todo: change fix value
            double pass = 35. / maxV;

            // check if not greater than Gmin
            if(pass > minGreenTime)
            {
                std::cout << "WARNING: Passage time is greater than Gmin in lane " << lane << endl;
                pass = minGreenTime;
            }

            // add this lane to the laneInfo map
            laneInfoEntry *entry = new laneInfoEntry(myTLid /*TLid*/, 0 /*initial detected time*/, pass /*initial passage time*/, std::map<std::string, queuedVehiclesEntry>() /*queued vehicles*/);
            laneInfo.insert( std::make_pair(lane, *entry) );
        }
    }
}


void ApplRSUTLVANET::finish()
{
    ApplRSUAID::finish();
}


void ApplRSUTLVANET::handleSelfMsg(cMessage* msg)
{
    ApplRSUAID::handleSelfMsg(msg);
}


void ApplRSUTLVANET::executeEachTimeStep(bool simulationDone)
{
    ApplRSUAID::executeEachTimeStep(simulationDone);

    if(collectVehApproach)
    {
        if(ev.isGUI()) saveVehApproach();    // (if in GUI) write what we have collected so far
        else if(simulationDone) saveVehApproach();  // (if in CMD) write to file at the end of simulation
    }
}


// draws a polygon to show the detection region
// it is for the purpose of region demonstration
void ApplRSUTLVANET::setDetectionRegion()
{
    Coord pos = TraCI->junctionGetPosition(myTLid);

    // todo: change from fix
    double x_length = 2*35 + 40;
    double y_length = 2*35 + 40;

    // draw detection region for junction TLid
    std::list<TraCICoord> detectionRegion;

    detectionRegion.push_back(TraCICoord( pos.x - x_length/2., pos.y + y_length/2.) );
    detectionRegion.push_back(TraCICoord( pos.x - x_length/2., pos.y - y_length/2.) );
    detectionRegion.push_back(TraCICoord( pos.x + x_length/2., pos.y - y_length/2.) );
    detectionRegion.push_back(TraCICoord( pos.x + x_length/2., pos.y + y_length/2.) );
    detectionRegion.push_back(TraCICoord( pos.x - x_length/2., pos.y + y_length/2.) );

    std::string polID = "detectionArea_" + myTLid;

    TraCI->polygonAdd(polID, "region", TraCIColor::fromTkColor("blue"), 0, 1, detectionRegion);
}


void ApplRSUTLVANET::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplRSUAID::onBeaconVehicle(wsm);

    if (TLControlMode == TL_MultiClass)
        onBeaconAny(wsm);
}


void ApplRSUTLVANET::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUAID::onBeaconBicycle(wsm);

    if (TLControlMode == TL_MultiClass)
        onBeaconAny(wsm);
}


void ApplRSUTLVANET::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUAID::onBeaconPedestrian(wsm);

    if (TLControlMode == TL_MultiClass)
        onBeaconAny(wsm);
}


void ApplRSUTLVANET::onBeaconRSU(BeaconRSU* wsm)
{
    ApplRSUAID::onBeaconRSU(wsm);
}


void ApplRSUTLVANET::onData(LaneChangeMsg* wsm)
{
    ApplRSUAID::onData(wsm);
}


// update variables upon reception of any beacon (vehicle, bike, pedestrian)
template <typename T> void ApplRSUTLVANET::onBeaconAny(T wsm)
{
    std::string sender = wsm->getSender();
    Coord pos = wsm->getPos();

    // If in the detection region:
    // todo: change from fix values
    // coordinates are the exact location of the middle of the LD
    if ( (pos.x >= 851.1 /*x-pos of west LD*/) && (pos.x <= 948.8 /*x-pos of east LD*/) && (pos.y >= 851.1 /*y-pos of north LD*/) && (pos.y <= 948.8 /*y-pos of north LD*/) )
    {
        std::string lane = wsm->getLane();

        // If on one of the incoming lanes:
        if(lanesTL.find(lane) != lanesTL.end() && lanesTL[lane] == myTLid)
        {
            // search queue for this vehicle
            const detectedVehicleEntry *searchFor = new detectedVehicleEntry(sender);
            std::vector<detectedVehicleEntry>::iterator counter = std::find(Vec_detectedVehicles.begin(), Vec_detectedVehicles.end(), *searchFor);

            // if we have already added it
            if (counter != Vec_detectedVehicles.end() && counter->leaveTime == -1)
            {
                return;
            }
            // the vehicle is visiting the intersection more than once
            else if (counter != Vec_detectedVehicles.end() && counter->leaveTime != -1)
            {
                counter->entryTime = simTime().dbl();
                counter->entrySpeed = wsm->getSpeed();
                counter->leaveTime = -1;
            }
            else
            {
                // Add entry:
                detectedVehicleEntry *tmp = new detectedVehicleEntry(sender, wsm->getSenderType(), lane, myTLid, simTime().dbl(), -1, wsm->getSpeed());
                Vec_detectedVehicles.push_back(*tmp);
            }

            // update laneInfo map
            std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(lane);
            if(loc != laneInfo.end())
            {
                // update detectedTime
                loc->second.lastDetectedTime = simTime().dbl();

                // add it as a queued vehicle on this lane
                queuedVehiclesEntry *newVeh = new queuedVehiclesEntry( wsm->getSenderType(), simTime().dbl(), wsm->getSpeed() );
                loc->second.queuedVehicles.insert( std::make_pair(sender, *newVeh) );
            }
            else
                error("lane %s does not exist in laneInfo map!", lane.c_str());

            // get the approach speed from the beacon
            double approachSpeed = wsm->getSpeed();
            // update passage time for this lane
            if(approachSpeed > 0)
            {
                // calculate passageTime for this lane
                // todo: change fix value
                double pass = 35. / approachSpeed;
                // check if not greater than Gmin
                if(pass > minGreenTime)
                    pass = minGreenTime;

                // update passage time
                std::map<std::string, laneInfoEntry>::iterator location = laneInfo.find(lane);
                location->second.passageTime = pass;
            }
        }
        // Else exiting queue area, so log leave time:
        else
        {
            // search queue for this vehicle
            const detectedVehicleEntry *searchFor = new detectedVehicleEntry(sender);
            std::vector<detectedVehicleEntry>::iterator counter = std::find(Vec_detectedVehicles.begin(), Vec_detectedVehicles.end(), *searchFor);

            if (counter == Vec_detectedVehicles.end())
                error("vehicle %s does not exist in the queue!", sender.c_str());

            if(counter->leaveTime == -1)
            {
                counter->leaveTime = simTime().dbl();

                // update laneInfo map
                std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(counter->lane);
                if(loc != laneInfo.end())
                {
                    // remove it from the queued vehicles
                    std::map<std::string, queuedVehiclesEntry>::iterator ref = loc->second.queuedVehicles.find(sender);
                    if(ref != loc->second.queuedVehicles.end())
                        loc->second.queuedVehicles.erase(ref);
                    else
                        error("vehicle %s was not added into lane %s in laneInfo map!", sender.c_str(), counter->lane.c_str());
                }
                else
                    error("lane %s does not exist in laneInfo map!", counter->lane.c_str());
            }
        }
    }
}


void ApplRSUTLVANET::saveVehApproach()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/vehApproach.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_vehApproach.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleType");
    fprintf (filePtr, "%-20s","lane");
    fprintf (filePtr, "%-20s","TLid");
    fprintf (filePtr, "%-20s","entryTime");
    fprintf (filePtr, "%-20s","entrySpeed");
    fprintf (filePtr, "%-20s\n\n","leaveTime");

    // write body
    for(std::vector<detectedVehicleEntry>::iterator y = Vec_detectedVehicles.begin(); y != Vec_detectedVehicles.end(); ++y)
    {
        fprintf (filePtr, "%-20s ", (*y).vehicleName.c_str());
        fprintf (filePtr, "%-20s ", (*y).vehicleType.c_str());
        fprintf (filePtr, "%-20s ", (*y).lane.c_str());
        fprintf (filePtr, "%-20s ", (*y).TLid.c_str());
        fprintf (filePtr, "%-20.2f ", (*y).entryTime);
        fprintf (filePtr, "%-20.2f", (*y).entrySpeed);
        fprintf (filePtr, "%-20.2f\n", (*y).leaveTime);
    }

    fclose(filePtr);
}

}
