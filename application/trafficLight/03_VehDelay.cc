/****************************************************************************/
/// @file    VehDelay.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    Jul 2015
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

#include <03_VehDelay.h>

namespace VENTOS {

Define_Module(VENTOS::VehDelay);


VehDelay::~VehDelay()
{

}


void VehDelay::initialize(int stage)
{
    LoopDetectors::initialize(stage);

    if(stage == 0)
    {
        measureVehDelay = par("measureVehDelay").boolValue();
        deccelDelayThreshold = par("deccelDelayThreshold").doubleValue();
        stoppingDelayThreshold = par("stoppingDelayThreshold").doubleValue();
        lastSpeedBuffSize = par("lastSpeedBuffSize").longValue();
        lastAccelBuffSize = par("lastAccelBuffSize").longValue();

        if(deccelDelayThreshold >= 0)
            error("deccelDelayThreshold is not set correctly!");

        if(stoppingDelayThreshold < 0)
            error("stoppingDelayThreshold is not set correctly!");

        if(lastSpeedBuffSize < 1)
            error("lastSpeedBuffSize is not set correctly!");

        if(lastAccelBuffSize < 1)
            error("lastAccelBuffSize is not set correctly!");
    }
}


void VehDelay::finish()
{
    LoopDetectors::finish();
}


void VehDelay::handleMessage(cMessage *msg)
{
    LoopDetectors::handleMessage(msg);
}


void VehDelay::executeFirstTimeStep()
{
    // call parent
    LoopDetectors::executeFirstTimeStep();

    // get all lanes in the network
    lanesList = TraCI->laneGetIDList();
}


void VehDelay::executeEachTimeStep(bool simulationDone)
{
    // call parent
    LoopDetectors::executeEachTimeStep(simulationDone);

    if(measureVehDelay)
    {
        vehiclesDelay();

        if(ev.isGUI()) vehiclesDelayToFile();   // (if in GUI) write what we have collected so far
        else if(simulationDone) vehiclesDelayToFile();  // (if in CMD) write to file at the end of simulation
    }
}


void VehDelay::vehiclesDelay()
{
    for(std::list<std::string>::iterator i = lanesList.begin(); i != lanesList.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> allVeh = TraCI->laneGetLastStepVehicleIDs( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = allVeh.rbegin(); k != allVeh.rend(); ++k)
        {
            std::string vID = k->c_str();

            // look for the vehicle in delay map
            std::map<std::string, delayEntry>::iterator loc = intersectionDelay.find(vID);

            // insert if new
            if(loc == intersectionDelay.end())
            {
                boost::circular_buffer<std::pair<double,double>> CB_speed;  // create a circular buffer
                CB_speed.set_capacity(lastSpeedBuffSize);                   // set max capacity
                CB_speed.clear();

                boost::circular_buffer<std::pair<double,double>> CB_speed2;  // create a circular buffer
                CB_speed2.set_capacity(lastSpeedBuffSize);                   // set max capacity
                CB_speed2.clear();

                boost::circular_buffer<std::pair<double,double>> CB_accel;  // create a circular buffer
                CB_accel.set_capacity(lastAccelBuffSize);                   // set max capacity
                CB_accel.clear();

                delayEntry *entry = new delayEntry("", "", -1, false, -1, -1, -1, -1, -1, -1, CB_speed, CB_speed2, CB_accel);
                intersectionDelay.insert( std::make_pair(vID, *entry) );
            }

            vehiclesDelayEach(vID);
        }
    }
}


void VehDelay::vehiclesDelayEach(std::string vID)
{
    // look for the vehicle in delay map
    std::map<std::string, delayEntry>::iterator loc = intersectionDelay.find(vID);

    // if the vehicle is controlled by a TL
    if(loc->second.TLid == "")
    {
        // get the TLid that controls this vehicle
        // empty string means the vehicle is not controlled by any TLid
        std::string result = TraCI->vehicleGetTLID(vID);

        if(result == "")
            return;
        else
            loc->second.TLid = result;
    }

    // if the veh is on its last lane before the intersection
    if(loc->second.lastLane == "")
    {
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // get best lanes for this vehicle
        std::map<int,bestLanesEntry> best = TraCI->vehicleGetBestLanes(vID);

        bool found = false;
        for(std::map<int,bestLanesEntry>::iterator y = best.begin(); y != best.end(); ++y)
        {
            std::string laneID = y->second.laneId;
            int offset = y->second.offset;
            int continuingDrive = y->second.continuingDrive;

            if(offset == 0 && continuingDrive == 1 && laneID == currentLane)
            {
                loc->second.lastLane = laneID;
                loc->second.intersectionEntrance = simTime().dbl();
                found = true;
                break;
            }
        }

        if(!found)
            return;
    }

    // check if we have crossed the intersection
    if(!loc->second.crossedIntersection)
    {
        // if passed the intersection
        if(TraCI->vehicleGetTLLinkStatus(vID) == 'n')
        {
            loc->second.crossedIntersection = true;
            loc->second.crossedTime = simTime().dbl();
        }
    }

    // looking for the start of deceleration delay
    if(loc->second.startDeccel == -1 && !loc->second.crossedIntersection)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        loc->second.lastSpeeds.push_back( std::make_pair(simTime().dbl(), speed) );

        double accel = TraCI->vehicleGetCurrentAccel(vID);
        loc->second.lastAccels.push_back( std::make_pair(simTime().dbl(), accel) );

        if(loc->second.lastAccels.full())
        {
            for(boost::circular_buffer<std::pair<double,double>>::iterator g = loc->second.lastAccels.begin(); g != loc->second.lastAccels.end(); ++g)
            {
                if( (*g).second > deccelDelayThreshold ) return;
            }

            loc->second.startDeccel = loc->second.lastAccels[0].first;
            loc->second.lastAccels.clear();

            loc->second.oldSpeed = loc->second.lastSpeeds[0].second;
            loc->second.lastSpeeds.clear();
        }
        else return;
    }

    // looking for the start of stopping delay
    // NOTE: stopping delay might be zero
    if(loc->second.startDeccel != -1 && loc->second.startStopping == -1 && !loc->second.crossedIntersection)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        loc->second.lastSpeeds.push_back( std::make_pair(simTime().dbl(), speed) );

        if(loc->second.lastSpeeds.full())
        {
            bool isStopped = true;
            for(boost::circular_buffer<std::pair<double,double>>::iterator g = loc->second.lastSpeeds.begin(); g != loc->second.lastSpeeds.end(); ++g)
            {
                if( (*g).second > stoppingDelayThreshold )
                {
                    isStopped = false;
                    break;
                }
            }

            if(isStopped)
            {
                loc->second.startStopping = loc->second.lastSpeeds[0].first;
                loc->second.lastSpeeds.clear();
            }
        }
    }

    // looking for the start of accel delay
    if(loc->second.startDeccel != -1 && loc->second.startAccel == -1 && loc->second.crossedIntersection)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        loc->second.lastSpeeds2.push_back( std::make_pair(simTime().dbl(), speed) );

        if(loc->second.lastSpeeds2.full())
        {
            for(boost::circular_buffer<std::pair<double,double>>::iterator g = loc->second.lastSpeeds2.begin(); g != loc->second.lastSpeeds2.end(); ++g)
            {
                if( (*g).second < stoppingDelayThreshold ) return;
            }

            loc->second.startAccel = loc->second.lastSpeeds2[0].first;
            loc->second.lastSpeeds2.clear();
        }
        else return;
    }

    // looking for the end of delay
    if(loc->second.startDeccel != -1 && loc->second.startAccel != -1 && loc->second.endDelay == -1 && loc->second.crossedIntersection)
    {
        if(TraCI->vehicleGetSpeed(vID) >= loc->second.oldSpeed)
            loc->second.endDelay = simTime().dbl();
        else return;
    }
}


void VehDelay::vehiclesDelayToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/vehicleDelay.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_vehicleDelay.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-15s","TLid");
    fprintf (filePtr, "%-20s","lastLane");
    fprintf (filePtr, "%-15s","entrance");
    fprintf (filePtr, "%-15s","crossed?");
    fprintf (filePtr, "%-15s","VbeforeDeccel");
    fprintf (filePtr, "%-15s","startDeccel");
    fprintf (filePtr, "%-15s","startStopping");
    fprintf (filePtr, "%-15s","crossedT");
    fprintf (filePtr, "%-15s","startAccel");
    fprintf (filePtr, "%-15s\n\n","endDelay");

    // write body
    for(std::map<std::string, delayEntry>::iterator y =  intersectionDelay.begin(); y != intersectionDelay.end(); ++y)
    {
        fprintf (filePtr, "%-20s", (*y).first.c_str());
        fprintf (filePtr, "%-15s", (*y).second.TLid.c_str());
        fprintf (filePtr, "%-20s", (*y).second.lastLane.c_str());
        fprintf (filePtr, "%-15.2f", (*y).second.intersectionEntrance);
        fprintf (filePtr, "%-15d", (*y).second.crossedIntersection);
        fprintf (filePtr, "%-15.2f", (*y).second.oldSpeed);
        fprintf (filePtr, "%-15.2f", (*y).second.startDeccel);
        fprintf (filePtr, "%-15.2f", (*y).second.startStopping);
        fprintf (filePtr, "%-15.2f", (*y).second.crossedTime);
        fprintf (filePtr, "%-15.2f", (*y).second.startAccel);
        fprintf (filePtr, "%-15.2f\n", (*y).second.endDelay);
    }

    fclose(filePtr);
}


}
