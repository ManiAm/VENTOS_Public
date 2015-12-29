/****************************************************************************/
/// @file    ApplRSU_02_Monitor.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Dec 2015
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

#include "ApplRSU_02_Monitor.h"

namespace VENTOS {

std::vector<detectedVehicleEntry> ApplRSUMonitor::Vec_detectedVehicles;

Define_Module(VENTOS::ApplRSUMonitor);

ApplRSUMonitor::~ApplRSUMonitor()
{

}


void ApplRSUMonitor::initialize(int stage)
{
    ApplRSUBase::initialize(stage);

    if (stage==0)
    {
        monitorVehApproach = par("monitorVehApproach").boolValue();
        collectVehApproach = par("collectVehApproach").boolValue();

        if(activeDetection)
        {
            monitorVehApproach = true;
            this->par("monitorVehApproach") = true;
        }
        else if(!monitorVehApproach)
            return;

        // we need this RSU to be associated with a TL
        if(myTLid == "")
            error("The id of %s does not match with any TL. Check RSUsLocation.xml file!", myFullId);

        // for each incoming lane in this TL
        std::list<std::string> lan = TraCI->TLGetControlledLanes(myTLid);

        // remove duplicate entries
        lan.unique();

        // for each incoming lane
        for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
            lanesTL[*it2] = myTLid;

        Vec_detectedVehicles.clear();

        setDetectionRegion();
    }
}


void ApplRSUMonitor::finish()
{
    ApplRSUBase::finish();
}


void ApplRSUMonitor::handleSelfMsg(cMessage* msg)
{
    ApplRSUBase::handleSelfMsg(msg);
}


void ApplRSUMonitor::executeEachTimeStep(bool simulationDone)
{
    ApplRSUBase::executeEachTimeStep(simulationDone);

    // (in CMD) write to file at the end of simulation
    if(monitorVehApproach && collectVehApproach && !ev.isGUI() && simulationDone)
        saveVehApproach();
}


// draws a polygon to show the detection region
// it is for the purpose of region demonstration
void ApplRSUMonitor::setDetectionRegion()
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

    TraCI->polygonAddTraCI(polID, "region", TraCIColor::fromTkColor("blue"), 0, 1, detectionRegion);
}


void ApplRSUMonitor::onBeaconVehicle(BeaconVehicle* wsm)
{
    if (monitorVehApproach)
        onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconBicycle(BeaconBicycle* wsm)
{
    if (monitorVehApproach)
        onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    if (monitorVehApproach)
        onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconRSU(BeaconRSU* wsm)
{

}


void ApplRSUMonitor::onData(LaneChangeMsg* wsm)
{

}


// update variables upon reception of any beacon (vehicle, bike, pedestrian)
template <typename T> void ApplRSUMonitor::onBeaconAny(T wsm)
{
    std::string sender = wsm->getSender();
    Coord pos = wsm->getPos();

    // If in the detection region:
    // todo: change from fix values
    // coordinates are the exact location of the middle of the LD
    if ( (pos.x >= 851.1 /*x-pos of west LD*/) && (pos.x <= 948.8 /*x-pos of east LD*/) && (pos.y >= 851.1 /*y-pos of north LD*/) && (pos.y <= 948.8 /*y-pos of north LD*/) )
    {
        std::string lane = wsm->getLane();

        // If on one of the incoming lanes
        if(lanesTL.find(lane) != lanesTL.end() && lanesTL[lane] == myTLid)
        {
            // search queue for this vehicle
            const detectedVehicleEntry *searchFor = new detectedVehicleEntry(sender);
            std::vector<detectedVehicleEntry>::iterator counter = std::find(Vec_detectedVehicles.begin(), Vec_detectedVehicles.end(), *searchFor);

            // return, if we have already added it
            if (counter != Vec_detectedVehicles.end() && counter->leaveTime == -1)
            {
                return;
            }
            // the vehicle is visiting the intersection more than once
            else if (counter != Vec_detectedVehicles.end() && counter->leaveTime != -1)
            {
                counter->entryTime = simTime().dbl();
                counter->entrySpeed = wsm->getSpeed();
                counter->pos = wsm->getPos();
                counter->leaveTime = -1;
            }
            else
            {
                // Add entry
                detectedVehicleEntry *tmp = new detectedVehicleEntry(sender, wsm->getSenderType(), lane, wsm->getPos(), myTLid, simTime().dbl(), -1, wsm->getSpeed());
                Vec_detectedVehicles.push_back(*tmp);
            }

            // save vehicle approach to file
            if(collectVehApproach && ev.isGUI())
                saveVehApproach();

            if (activeDetection)
                UpdateLaneInfoAdd(lane, sender, wsm->getSenderType(), wsm->getSpeed());
        }
        // Else exiting queue area, so log leave time
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

                // save vehicle approach to file
                if(collectVehApproach && ev.isGUI())
                    saveVehApproach();

                if (activeDetection)
                    UpdateLaneInfoRemove(counter->lane, sender);
            }
        }
    }
}


void ApplRSUMonitor::saveVehApproach()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/vehApproach.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_vehApproach.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!ev.isGUI())
    {
        // get the current config name
        std::string configName = ev.getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = ev.getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = ev.getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleType");
    fprintf (filePtr, "%-15s","lane");
    fprintf (filePtr, "%-15s","posX");
    fprintf (filePtr, "%-15s","posY");
    fprintf (filePtr, "%-15s","TLid");
    fprintf (filePtr, "%-15s","entryTime");
    fprintf (filePtr, "%-15s","entrySpeed");
    fprintf (filePtr, "%-15s\n\n","leaveTime");

    // write body
    for(std::vector<detectedVehicleEntry>::iterator y = Vec_detectedVehicles.begin(); y != Vec_detectedVehicles.end(); ++y)
    {
        fprintf (filePtr, "%-20s ", (*y).vehicleName.c_str());
        fprintf (filePtr, "%-20s ", (*y).vehicleType.c_str());
        fprintf (filePtr, "%-13s ", (*y).lane.c_str());
        fprintf (filePtr, "%-15.2f ", (*y).pos.x);
        fprintf (filePtr, "%-15.2f ", (*y).pos.y);
        fprintf (filePtr, "%-15s ", (*y).TLid.c_str());
        fprintf (filePtr, "%-15.2f ", (*y).entryTime);
        fprintf (filePtr, "%-15.2f", (*y).entrySpeed);
        fprintf (filePtr, "%-15.2f\n", (*y).leaveTime);
    }

    fclose(filePtr);
}


void ApplRSUMonitor::UpdateLaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed)
{
    error("this method can not be called directly!");
}


void ApplRSUMonitor::UpdateLaneInfoRemove(std::string counter, std::string sender)
{
    error("this method can not be called directly!");
}

}
