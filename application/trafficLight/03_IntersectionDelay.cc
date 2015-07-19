/****************************************************************************/
/// @file    IntersectionDelay.cc
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

#include <03_IntersectionDelay.h>

namespace VENTOS {

Define_Module(VENTOS::IntersectionDelay);


IntersectionDelay::~IntersectionDelay()
{

}


void IntersectionDelay::initialize(int stage)
{
    LoopDetectors::initialize(stage);

    if(stage == 0)
    {
        collectVehDelay = par("collectVehDelay").boolValue();
        deccelDelayThreshold = par("deccelDelayThreshold").doubleValue();
        stoppingDelayThreshold = par("stoppingDelayThreshold").doubleValue();
        lastValueBuffSize = par("lastValueBuffSize").longValue();

        if(deccelDelayThreshold >= 0)
            error("deccelDelayThreshold is not set correctly!");

        if(stoppingDelayThreshold < 0)
            error("stoppingDelayThreshold is not set correctly!");

        if(lastValueBuffSize < 1)
            error("lastValueBuffSize is not set correctly!");
    }
}


void IntersectionDelay::finish()
{
    LoopDetectors::finish();
}


void IntersectionDelay::handleMessage(cMessage *msg)
{
    LoopDetectors::handleMessage(msg);
}


void IntersectionDelay::executeFirstTimeStep()
{
    // call parent
    LoopDetectors::executeFirstTimeStep();

    // get all lanes in the network
    lanesList = TraCI->laneGetIDList();

    for(std::map<std::string,std::string>::iterator it = lanesTL.begin(); it != lanesTL.end(); ++it)
    {
        std::string lane = (*it).first;
        std::string TLid = (*it).second;

        // initialize laneDelay
        laneDelay[lane] = std::map<std::string,double> ();
    }

}


void IntersectionDelay::executeEachTimeStep(bool simulationDone)
{
    // call parent
    LoopDetectors::executeEachTimeStep(simulationDone);

    if(collectVehDelay)
    {
        vehiclesDelay();

        if(ev.isGUI()) vehiclesDelayToFile();   // (if in GUI) write what we have collected so far
        else if(simulationDone) vehiclesDelayToFile();  // (if in CMD) write to file at the end of simulation
    }
}


void IntersectionDelay::vehiclesDelay()
{
    for(std::list<std::string>::iterator i = lanesList.begin(); i != lanesList.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> allVeh = TraCI->laneGetLastStepVehicleIDs( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = allVeh.rbegin(); k != allVeh.rend(); ++k)
        {
            std::string vID = k->c_str();

            // look for the vehicle in delay map
            std::map<std::string, delayEntry>::iterator loc = vehDelay.find(vID);

            // todo: what if the vehicle is visiting this TL for the second time?
            // insert if new
            if(loc == vehDelay.end())
            {
                boost::circular_buffer<std::pair<double,double>> CB_speed;  // create a circular buffer
                CB_speed.set_capacity(lastValueBuffSize);                   // set max capacity
                CB_speed.clear();

                boost::circular_buffer<std::pair<double,double>> CB_speed2;  // create a circular buffer
                CB_speed2.set_capacity(lastValueBuffSize);                   // set max capacity
                CB_speed2.clear();

                boost::circular_buffer<std::pair<double,double>> CB_accel;  // create a circular buffer
                CB_accel.set_capacity(lastValueBuffSize);                   // set max capacity
                CB_accel.clear();

                boost::circular_buffer<char> CB_sig;                        // create a circular buffer
                CB_sig.set_capacity(lastValueBuffSize);                     // set max capacity
                CB_sig.clear();

                delayEntry *entry = new delayEntry("" /*TLid*/, "" /*lastLane*/, -1 /*entrance*/, false /*crossed?*/, -1 /*crossedT*/,
                        -1 /*old speed*/, -1 /*startDeccel*/, -1 /*startStopping*/, -1 /*startAccel*/, -1 /*endDelay*/, 0 /*accumulated delay so far*/,
                        CB_speed, CB_speed2, CB_accel, CB_sig);
                vehDelay.insert( std::make_pair(vID, *entry) );
            }

            vehiclesDelayEach(vID);
        }
    }
}


void IntersectionDelay::vehiclesDelayEach(std::string vID)
{
    // look for the vehicle in delay map
    std::map<std::string, delayEntry>::iterator loc = vehDelay.find(vID);

    // if the vehicle is controlled by a TL
    if(loc->second.TLid == "")
    {
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // If on one of the incoming lanes:
        if(lanesTL.find(currentLane) != lanesTL.end())
            loc->second.TLid = lanesTL[currentLane];
        else return;
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
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // If we are at the middle of intersection
        if(lanesTL.find(currentLane) == lanesTL.end())
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

        char signal = TraCI->vehicleGetTLLinkStatus(vID);
        loc->second.lastSignals.push_back(signal);

        if(loc->second.lastAccels.full())
        {
            for(boost::circular_buffer<std::pair<double,double>>::iterator g = loc->second.lastAccels.begin(); g != loc->second.lastAccels.end(); ++g)
            {
                if( (*g).second > deccelDelayThreshold ) return;
            }

            // At this point a slow down is found.
            // We should check if this slow down was due to a Y or R signal
            bool YorR = false;
            for(boost::circular_buffer<char>::iterator g = loc->second.lastSignals.begin(); g != loc->second.lastSignals.end(); ++g)
            {
                if(*g == 'y' || *g == 'r')
                {
                    YorR = true;
                    break;
                }
            }

            if(YorR)
            {
                loc->second.startDeccel = loc->second.lastAccels[0].first;
                loc->second.lastAccels.clear();

                loc->second.oldSpeed = loc->second.lastSpeeds[0].second;
                loc->second.lastSpeeds.clear();

                loc->second.lastSignals.clear();
            }
            else return;
        }
        else return;
    }

    // update accumulated delay for this vehicle
    vehiclesAccuDelay(vID, loc);

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


void IntersectionDelay::vehiclesAccuDelay(std::string vID, std::map<std::string, delayEntry>::iterator loc)
{
    if(!loc->second.crossedIntersection)
    {
        loc->second.accumDelay = simTime().dbl() - loc->second.startDeccel;

        if(loc->second.accumDelay < 0)
            error("accumulated delay can not be negative for vehicle %s", vID.c_str());

        if(loc->second.accumDelay > simTime().dbl())
            error("accumulated delay can not be greater than the current simTime for vehicle %s", vID.c_str());

        // update accumDelay of this vehicle in laneDelay map
        std::map<std::string,double>::iterator loc2 = laneDelay[loc->second.lastLane].find(vID);
        if(loc2 == laneDelay[loc->second.lastLane].end())
            laneDelay[loc->second.lastLane].insert(std::make_pair(vID,loc->second.accumDelay));
        else
            (*loc2).second = loc->second.accumDelay;

        // iterate over outgoing links
        std::pair<std::multimap<std::string, std::pair<std::string, int>>::iterator, std::multimap<std::string, std::pair<std::string, int>>::iterator > ppp;
        ppp = linksTL.equal_range(loc->second.lastLane);
        for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
        {
            int linkNumber = (*z).second.second;

            std::map<std::string,double>::iterator loc3 = linkDelay[std::make_pair(loc->second.TLid,linkNumber)].find(vID);
            if(loc3 == linkDelay[std::make_pair(loc->second.TLid,linkNumber)].end())
                linkDelay[std::make_pair(loc->second.TLid,linkNumber)].insert(std::make_pair(vID,loc->second.accumDelay));
            else
                (*loc3).second = loc->second.accumDelay;
        }
    }
    // as soon as the vehicle crossed the intersection, we set accumulated delay to zero
    else
    {
        loc->second.accumDelay = 0;

        // remove the vehicle from laneDelay (if exists)
        std::map<std::string,double>::iterator loc2 = laneDelay[loc->second.lastLane].find(vID);
        if(loc2 != laneDelay[loc->second.lastLane].end())
            laneDelay[loc->second.lastLane].erase(loc2);

        // iterate over outgoing links
        std::pair<std::multimap<std::string, std::pair<std::string, int>>::iterator, std::multimap<std::string, std::pair<std::string, int>>::iterator > ppp;
        ppp = linksTL.equal_range(loc->second.lastLane);
        for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
        {
            int linkNumber = (*z).second.second;

            std::map<std::string,double>::iterator loc3 = linkDelay[std::make_pair(loc->second.TLid,linkNumber)].find(vID);
            if(loc3 != linkDelay[std::make_pair(loc->second.TLid,linkNumber)].end())
                linkDelay[std::make_pair(loc->second.TLid,linkNumber)].erase(loc3);
        }
    }
}


void IntersectionDelay::vehiclesDelayToFile()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/vehDelay.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_vehDelay.txt";
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
    fprintf (filePtr, "%-15s","vehicleName");
    fprintf (filePtr, "%-10s","TLid");
    fprintf (filePtr, "%-10s","lastLane");
    fprintf (filePtr, "%-15s","entrance");
    fprintf (filePtr, "%-10s","crossed?");
    fprintf (filePtr, "%-15s","VbeforeDeccel");
    fprintf (filePtr, "%-15s","startDeccel");
    fprintf (filePtr, "%-15s","startStopping");
    fprintf (filePtr, "%-15s","crossedT");
    fprintf (filePtr, "%-15s","startAccel");
    fprintf (filePtr, "%-15s\n\n","endDelay");

    // write body
    for(std::map<std::string, delayEntry>::iterator y =  vehDelay.begin(); y != vehDelay.end(); ++y)
    {
        fprintf (filePtr, "%-15s", (*y).first.c_str());
        fprintf (filePtr, "%-10s", (*y).second.TLid.c_str());
        fprintf (filePtr, "%-10s", (*y).second.lastLane.c_str());
        fprintf (filePtr, "%-15.2f", (*y).second.intersectionEntrance);
        fprintf (filePtr, "%-10d", (*y).second.crossedIntersection);
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
