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

#include <iomanip>

#include "trafficLight/06_IntersectionDelay.h"

namespace VENTOS {

Define_Module(VENTOS::IntersectionDelay);


IntersectionDelay::~IntersectionDelay()
{

}


void IntersectionDelay::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        collectVehDelay = par("collectVehDelay").boolValue();
        deccelDelayThreshold = par("deccelDelayThreshold").doubleValue();
        vehStoppingDelayThreshold = par("vehStoppingDelayThreshold").doubleValue();
        bikeStoppingDelayThreshold = par("bikeStoppingDelayThreshold").doubleValue();
        lastValueBuffSize = par("lastValueBuffSize").longValue();

        if(deccelDelayThreshold >= 0)
            throw omnetpp::cRuntimeError("deccelDelayThreshold is not set correctly!");

        if(vehStoppingDelayThreshold < 0)
            throw omnetpp::cRuntimeError("vehStoppingDelayThreshold is not set correctly!");

        if(bikeStoppingDelayThreshold < 0)
            throw omnetpp::cRuntimeError("bikeStoppingDelayThreshold is not set correctly!");

        if(lastValueBuffSize < 1)
            throw omnetpp::cRuntimeError("lastValueBuffSize is not set correctly!");
    }
}


void IntersectionDelay::finish()
{
    super::finish();

    if(collectVehDelay)
        vehiclesDelayToFile();
}


void IntersectionDelay::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void IntersectionDelay::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    // get all lanes in the network
    lanesList = TraCI->laneGetIDList();

    for(auto &it : allIncomingLanes)
    {
        std::string lane = it.first;
        std::string TLid = it.second;

        // initialize laneDelay
        laneDelay[lane] = std::map<std::string,double> ();
    }
}


void IntersectionDelay::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(collectVehDelay)
        vehiclesDelay();
}


void IntersectionDelay::vehiclesDelay()
{
    for(auto &i : lanesList)
    {
        // get all vehicles on lane i
        std::list<std::string> allVeh = TraCI->laneGetLastStepVehicleIDs( i.c_str() );

        for(std::list<std::string>::reverse_iterator k = allVeh.rbegin(); k != allVeh.rend(); ++k)
        {
            std::string vID = k->c_str();

            // look for the vehicle in delay map
            auto loc = vehDelay.find(vID);

            // todo: what if the vehicle is visiting this TL for the second time?
            // insert if new
            if(loc == vehDelay.end())
            {
                std::string vehType = TraCI->vehicleGetTypeID(vID);

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

                delayEntry *entry = new delayEntry(vehType, "" /*TLid*/, "" /*lastLane*/, -1 /*entrance*/, false /*crossed?*/, -1 /*crossedT*/,
                        -1 /*old speed*/, -1 /*startDeccel*/, -1 /*startStopping*/, -1 /*startAccel*/, -1 /*endDelay*/,
                        0 /*decel delay*/, 0 /*waiting delay*/, 0 /*accumulated delay so far*/,
                        CB_speed, CB_speed2, CB_accel, CB_sig);
                vehDelay.insert( std::make_pair(vID, *entry) );
            }

            vehiclesDelayStart(vID);
            vehiclesDelayDuration(vID);
        }
    }
}


void IntersectionDelay::vehiclesDelayStart(std::string vID)
{
    // get a pointer to the vehicle
    auto loc = vehDelay.find(vID);

    // if the vehicle is controlled by a TL
    if(loc->second.TLid == "")
    {
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // If on one of the incoming lanes:
        auto itl = allIncomingLanes.find(currentLane);
        if(itl != allIncomingLanes.end())
            loc->second.TLid = itl->second;
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
        for(auto &y : best)
        {
            std::string laneID = y.second.laneId;
            int offset = y.second.offset;
            int continuingDrive = y.second.continuingDrive;

            if(offset == 0 && continuingDrive == 1 && laneID == currentLane)
            {
                loc->second.lastLane = laneID;
                loc->second.intersectionEntrance = omnetpp::simTime().dbl();

                if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
                {
                    printf("*** %-10s is approaching TL %s on lane %s \n", vID.c_str(), loc->second.TLid.c_str(), laneID.c_str());
                    std::cout.flush();
                }

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
        if(allIncomingLanes.find(currentLane) == allIncomingLanes.end())
        {
            loc->second.crossedIntersection = true;
            loc->second.crossedTime = omnetpp::simTime().dbl();

            if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
            {
                printf("*** %-10s crossed TL %s on lane %s \n", vID.c_str(), loc->second.TLid.c_str(), loc->second.lastLane.c_str());
                std::cout.flush();
            }
        }
    }

    // looking for the start of deceleration delay
    if(loc->second.startDeccel == -1 && !loc->second.crossedIntersection)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        loc->second.lastSpeeds.push_back( std::make_pair(omnetpp::simTime().dbl(), speed) );

        double accel = TraCI->vehicleGetCurrentAccel(vID);
        loc->second.lastAccels.push_back( std::make_pair(omnetpp::simTime().dbl(), accel) );

        char signal = TraCI->vehicleGetTLLinkStatus(vID);
        loc->second.lastSignals.push_back(signal);

        if(loc->second.lastAccels.full())
        {
            for(auto &g : loc->second.lastAccels)
                if( g.second > deccelDelayThreshold ) return;

            // At this point a slow down is found.
            // We should check if this slow down was due to a Y or R signal
            bool YorR = false;
            for(auto &g : loc->second.lastSignals)
            {
                if(g == 'y' || g == 'r')
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

                if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
                {
                    printf("*** %-10s is decelerating on lane %s \n", vID.c_str(), loc->second.lastLane.c_str());
                    std::cout.flush();
                }
            }
            else return;
        }
        else return;
    }

    double stoppingDelayThreshold = 0;
    if(loc->second.vehType == "bicycle")
        stoppingDelayThreshold = bikeStoppingDelayThreshold;
    else
        stoppingDelayThreshold = vehStoppingDelayThreshold;

    // looking for the start of stopping delay
    // NOTE: stopping delay might be zero
    if(loc->second.startDeccel != -1 && loc->second.startStopping == -1 && !loc->second.crossedIntersection)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        loc->second.lastSpeeds.push_back( std::make_pair(omnetpp::simTime().dbl(), speed) );

        if(loc->second.lastSpeeds.full())
        {
            bool isStopped = true;
            for(auto& g : loc->second.lastSpeeds)
            {
                if( g.second > stoppingDelayThreshold )
                {
                    isStopped = false;
                    break;
                }
            }

            if(isStopped)
            {
                loc->second.startStopping = loc->second.lastSpeeds[0].first;
                loc->second.lastSpeeds.clear();

                if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
                {
                    printf("*** %-10s is stopping on lane %s. \n", vID.c_str(), loc->second.lastLane.c_str());
                    std::cout.flush();
                }
            }
        }
    }

    // looking for the start of accel delay
    if(loc->second.startDeccel != -1 && loc->second.startAccel == -1 && loc->second.crossedIntersection)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        loc->second.lastSpeeds2.push_back( std::make_pair(omnetpp::simTime().dbl(), speed) );

        if(loc->second.lastSpeeds2.full())
        {
            for(auto &g : loc->second.lastSpeeds2)
            {
                if( g.second < stoppingDelayThreshold ) return;
            }

            loc->second.startAccel = loc->second.lastSpeeds2[0].first;
            loc->second.lastSpeeds2.clear();

            if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
            {
                printf("*** %-10s is accelerating on lane %s. \n", vID.c_str(), loc->second.lastLane.c_str());
                std::cout.flush();
            }
        }
        else return;
    }

    // looking for the end of delay
    if(loc->second.startDeccel != -1 && loc->second.startAccel != -1 && loc->second.endDelay == -1 && loc->second.crossedIntersection)
    {
        if(TraCI->vehicleGetSpeed(vID) >= loc->second.oldSpeed)
            loc->second.endDelay = omnetpp::simTime().dbl();
        else return;
    }
}


void IntersectionDelay::vehiclesDelayDuration(std::string vID)
{
    // get a pointer to the vehicle
    auto loc = vehDelay.find(vID);

    if(loc->second.startDeccel == -1)
        return;

    // as long as the vehicle does not cross the intersection
    if(!loc->second.crossedIntersection)
    {
        // update deceleration delay duration
        if(loc->second.startStopping != -1)
            loc->second.decelDelay = loc->second.startStopping - loc->second.startDeccel;
        else if(loc->second.startAccel != -1)
            loc->second.decelDelay = loc->second.startAccel - loc->second.startDeccel;
        else
            loc->second.decelDelay = omnetpp::simTime().dbl() - loc->second.startDeccel;
        if(loc->second.decelDelay < 0 || loc->second.decelDelay > omnetpp::simTime().dbl())
            throw omnetpp::cRuntimeError("deceleration delay value is incorrect for vehicle %s", vID.c_str());

        // update waiting delay duration
        if(loc->second.startStopping != -1)
        {
            if(loc->second.startAccel != -1)
                loc->second.waitingDelay = loc->second.startAccel - loc->second.startStopping;
            else
                loc->second.waitingDelay = omnetpp::simTime().dbl() - loc->second.startStopping;
            if(loc->second.waitingDelay < 0 || loc->second.waitingDelay > omnetpp::simTime().dbl())
                throw omnetpp::cRuntimeError("waiting delay value is incorrect for vehicle %s", vID.c_str());
        }

        // update accumulated delay duration
        loc->second.accumDelay = omnetpp::simTime().dbl() - loc->second.startDeccel;
        if(loc->second.accumDelay < 0 || loc->second.accumDelay > omnetpp::simTime().dbl())
            throw omnetpp::cRuntimeError("accumulated delay value is incorrect for vehicle %s", vID.c_str());

        // update accumDelay of this vehicle in laneDelay
        auto loc2 = laneDelay[loc->second.lastLane].find(vID);
        if(loc2 == laneDelay[loc->second.lastLane].end())
            laneDelay[loc->second.lastLane].insert(std::make_pair(vID,loc->second.accumDelay));
        else
            (*loc2).second = loc->second.accumDelay;

        // iterate over outgoing links
        auto ppp = outgoingLinks.equal_range(loc->second.lastLane);
        for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
        {
            int linkNumber = (*z).second.second;

            auto loc3 = linkDelay[std::make_pair(loc->second.TLid,linkNumber)].find(vID);
            if(loc3 == linkDelay[std::make_pair(loc->second.TLid,linkNumber)].end())
                linkDelay[std::make_pair(loc->second.TLid,linkNumber)].insert(std::make_pair(vID,loc->second.accumDelay));
            else
                (*loc3).second = loc->second.accumDelay;
        }
    }
    // as soon as the vehicle crossed the intersection, we reset the delays
    else
    {
        loc->second.decelDelay = 0;
        loc->second.waitingDelay = 0;
        loc->second.accumDelay = 0;

        // remove the vehicle from laneDelay (if exists)
        std::map<std::string,double>::iterator loc2 = laneDelay[loc->second.lastLane].find(vID);
        if(loc2 != laneDelay[loc->second.lastLane].end())
            laneDelay[loc->second.lastLane].erase(loc2);

        // iterate over outgoing links
        auto ppp = outgoingLinks.equal_range(loc->second.lastLane);
        for(std::multimap<std::string, std::pair<std::string, int>>::iterator z = ppp.first; z != ppp.second; ++z)
        {
            int linkNumber = (*z).second.second;

            auto loc3 = linkDelay[std::make_pair(loc->second.TLid,linkNumber)].find(vID);
            if(loc3 != linkDelay[std::make_pair(loc->second.TLid,linkNumber)].end())
                linkDelay[std::make_pair(loc->second.TLid,linkNumber)].erase(loc3);
        }
    }
}


void IntersectionDelay::vehiclesDelayToFile()
{
    if(vehDelay.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_vehDelay.txt") % currentRun;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[currentRun].c_str());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleType");
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
    for(auto &y : vehDelay)
    {
        fprintf (filePtr, "%-20s", y.first.c_str());
        fprintf (filePtr, "%-20s", y.second.vehType.c_str());
        fprintf (filePtr, "%-10s", y.second.TLid.c_str());
        fprintf (filePtr, "%-10s", y.second.lastLane.c_str());
        fprintf (filePtr, "%-15.2f", y.second.intersectionEntrance);
        fprintf (filePtr, "%-10d", y.second.crossedIntersection);
        fprintf (filePtr, "%-15.2f", y.second.oldSpeed);
        fprintf (filePtr, "%-15.2f", y.second.startDeccel);
        fprintf (filePtr, "%-15.2f", y.second.startStopping);
        fprintf (filePtr, "%-15.2f", y.second.crossedTime);
        fprintf (filePtr, "%-15.2f", y.second.startAccel);
        fprintf (filePtr, "%-15.2f\n", y.second.endDelay);
    }

    fclose(filePtr);
}


}
