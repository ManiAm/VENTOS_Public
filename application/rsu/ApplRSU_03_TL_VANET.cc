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

    if (TLControlMode != TL_VANET)
        return;

    if (stage==0)
    {
        // TraCI connection is established here (cause AddRSU invoked this)
        // for each traffic light, get all incoming lanes
        std::list<std::string> TLList = TraCI->TLGetIDList();
        for (std::list<std::string>::iterator it = TLList.begin(); it != TLList.end(); ++it)
        {
            // for lane
            std::list<std::string> lan = TraCI->TLGetControlledLanes(*it);

            // remove duplicate entries
            lan.unique();

            // for each incoming lane
            for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
            {
                lanesTL[*it2] = *it;
            }
        }

        // initialize all detectedTime with zero
        for(std::map<std::string, std::string>::iterator y = lanesTL.begin(); y != lanesTL.end(); ++y)
        {
            std::string lane = (*y).first;

            detectedTime[lane] = 0;
        }

        setDetectionRegion();
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
}


void ApplRSUTLVANET::setDetectionRegion()
{
    // draw detection region for TL 'C'
    std::list<TraCICoord> detectionRegion;
    detectionRegion.push_back(TraCICoord(350, 350));
    detectionRegion.push_back(TraCICoord(350, 450));
    detectionRegion.push_back(TraCICoord(450, 450));
    detectionRegion.push_back(TraCICoord(450, 350));
    detectionRegion.push_back(TraCICoord(350, 350));
    TraCI->polygonAdd("detectionArea_C", "region", TraCIColor::fromTkColor("blue"), 0, 1, detectionRegion);
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
    if ( (pos.x > 350) && (pos.x < 450) && (pos.y > 350) && (pos.y < 450) )
    {
        std::string lane = wsm->getLane();

        // If on one the incoming lanes:
        if(lanesTL.find(lane) != lanesTL.end() && lanesTL[lane] == "C")
        {
            // search queue for this vehicle
            const queueData *searchFor = new queueData(sender, "");
            std::vector<queueData>::iterator counter = std::find(Vec_queueData.begin(), Vec_queueData.end(), *searchFor);

            // If not in queue
            if (counter == Vec_queueData.end())
            {
                // Add entry:
                queueData *tmp = new queueData(sender, lane, simTime().dbl(), -1, wsm->getSpeed());
                Vec_queueData.push_back(*tmp);

                std::cout << myFullId << ": vehicle '" << sender << "' is approaching from lane '" << lane << "' to the intersection 'C'." << endl;

                // also update detectedTime (used by the TL_VANET)
                std::map<std::string, double>::iterator loc = detectedTime.find(lane);
                if(loc != detectedTime.end())
                    loc->second = simTime().dbl();
                else
                    error("lane %s does not exist in detectedTime map!", lane.c_str());
            }
        }
        // Else exiting queue area, so log leave time:
        else
        {
            // search queue for this vehicle
            const queueData *searchFor = new queueData(sender, "");
            std::vector<queueData>::iterator counter = std::find(Vec_queueData.begin(), Vec_queueData.end(), *searchFor);

            if (counter == Vec_queueData.end())
                error("vehicle %s does not exist in the queue!", sender.c_str());

            if(counter->leaveTime == -1)
            {
                counter->leaveTime = simTime().dbl();

                std::cout << myFullId << ": vehicle '"  << sender << " left the intersection."
                        << " EntryT: " << counter->entryTime
                        << " | LeaveT: " << counter->leaveTime
                        << " | Speed: "  << counter->entrySpeed  << endl;
            }
        }
    }
}

}
