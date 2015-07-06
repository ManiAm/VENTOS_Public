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

std::vector<queueData> ApplRSUTLVANET::Vec_queueData;

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

    if (TLControlMode != TL_VANET)
        return;

    if (stage==0)
    {
        if(myTLid == "")
            error("The id of %s does not match with any TL. Check RSUsLocation.xml file!", myFullId);

        collectVehApproach = par("collectVehApproach").boolValue();

        Vec_queueData.clear();

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

            // initialize all detectedTime with zero
            detectedTime[lane] = 0;

            // get the max speed on this lane
            double maxV = TraCI->laneGetMaxSpeed(lane);

            // calculate passageTime for this lane
            // todo: change fix value
            double pass = 35. / maxV;

            // check if not greater than Gmin
            if(pass > minGreenTime)
            {
                std::cout << "WARNING: Passage time is greater than Gmin in lane " << lane << endl;
                pass = minGreenTime;
            }

            // set it for each lane
            passageTimePerLane[lane] = pass;
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


void ApplRSUTLVANET::setDetectionRegion()
{
    Coord pos = TraCI->junctionGetPosition(myTLid);

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

    if (TLControlMode == TL_VANET)
        onBeaconAny(wsm);
}


void ApplRSUTLVANET::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUAID::onBeaconBicycle(wsm);

    if (TLControlMode == TL_VANET)
        onBeaconAny(wsm);
}


void ApplRSUTLVANET::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUAID::onBeaconPedestrian(wsm);

    if (TLControlMode == TL_VANET)
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


template <typename T> void ApplRSUTLVANET::onBeaconAny(T wsm)
{
    std::string sender = wsm->getSender();
    Coord pos = wsm->getPos();

    // If in the detection region:
    // todo: change from fix values
    if ( (pos.x > 845) && (pos.x < 955) && (pos.y > 843) && (pos.y < 955) )
    {
        std::string lane = wsm->getLane();

        // If on one of the incoming lanes:
        if(lanesTL.find(lane) != lanesTL.end() && lanesTL[lane] == myTLid)
        {
            // search queue for this vehicle
            const queueData *searchFor = new queueData(sender);
            std::vector<queueData>::iterator counter = std::find(Vec_queueData.begin(), Vec_queueData.end(), *searchFor);

            // If not in queue
            if (counter == Vec_queueData.end())
            {
                // Add entry:
                queueData *tmp = new queueData(sender, lane, myTLid, simTime().dbl(), -1, wsm->getSpeed());
                Vec_queueData.push_back(*tmp);

                // also update detectedTime (used by the TL_VANET)
                std::map<std::string, double>::iterator loc = detectedTime.find(lane);
                if(loc != detectedTime.end())
                    loc->second = simTime().dbl();
                else
                    error("lane %s does not exist in detectedTime map!", lane.c_str());
            }

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

                // update passage time value in passageTimePerLane
                std::map<std::string,double>::iterator location = passageTimePerLane.find(lane);
                location->second = pass;
            }
        }
        // Else exiting queue area, so log leave time:
        else
        {
            // search queue for this vehicle
            const queueData *searchFor = new queueData(sender);
            std::vector<queueData>::iterator counter = std::find(Vec_queueData.begin(), Vec_queueData.end(), *searchFor);

            if (counter == Vec_queueData.end())
                error("vehicle %s does not exist in the queue!", sender.c_str());

            if(counter->leaveTime == -1)
                counter->leaveTime = simTime().dbl();
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
    fprintf (filePtr, "%-20s","lane");
    fprintf (filePtr, "%-20s","TLid");
    fprintf (filePtr, "%-20s","entryTime");
    fprintf (filePtr, "%-20s","entrySpeed");
    fprintf (filePtr, "%-20s\n\n","leaveTime");

    // write body
    for(std::vector<queueData>::iterator y = Vec_queueData.begin(); y != Vec_queueData.end(); ++y)
    {
        fprintf (filePtr, "%-20s ", (*y).vehicleName.c_str());
        fprintf (filePtr, "%-20s ", (*y).lane.c_str());
        fprintf (filePtr, "%-20s ", (*y).TLid.c_str());
        fprintf (filePtr, "%-20.2f ", (*y).entryTime);
        fprintf (filePtr, "%-20.2f", (*y).entrySpeed);
        fprintf (filePtr, "%-20.2f\n", (*y).leaveTime);
    }

    fclose(filePtr);
}

}
