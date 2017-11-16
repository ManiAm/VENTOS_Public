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

#include "trafficLight/04_IntersectionDelay.h"

namespace VENTOS {

Define_Module(VENTOS::IntersectionDelay);


IntersectionDelay::~IntersectionDelay()
{
    for(auto &item : vehDelay_perTL)
        for(auto &item2 : item.second)
            delete item2;
}


void IntersectionDelay::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        record_intersectionDelay_stat = par("record_intersectionDelay_stat").boolValue();

        speedThreshold_veh = par("speedThreshold_veh").doubleValue();
        speedThreshold_bike = par("speedThreshold_bike").doubleValue();

        if(speedThreshold_veh < 0)
            throw omnetpp::cRuntimeError("speedThreshold_veh is not set correctly!");

        if(speedThreshold_bike < 0)
            throw omnetpp::cRuntimeError("speedThreshold_bike is not set correctly!");

        deccelDelayThreshold = par("deccelDelayThreshold").doubleValue();
        if(deccelDelayThreshold >= 0)
            throw omnetpp::cRuntimeError("deccelDelayThreshold is not set correctly!");

        Signal_arrived_vehs = registerSignal("vehicleArrivedSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("vehicleArrivedSignal", this);
    }
}


void IntersectionDelay::finish()
{
    super::finish();

    vehiclesDelayToFile();
}


void IntersectionDelay::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void IntersectionDelay::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, const char *SUMOID, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_arrived_vehs)
    {
        // remove the arrived vehicle entry from the map
        auto it = vehDelay_perTL.find(SUMOID);
        if(it != vehDelay_perTL.end())
        {
            for(auto &e : it->second)
            {
                e->lastSpeeds.clear();
                e->lastSpeeds2.clear();
                e->lastAccels.clear();
                e->lastSignals.clear();

                delete e;
            }

            vehDelay_perTL.erase(it);
        }
    }
}


void IntersectionDelay::initialize_withTraCI()
{
    super::initialize_withTraCI();

    if(record_intersectionDelay_stat)
    {
        TLList = TraCI->TLGetIDList();

        if(TLList.empty())
            LOG_INFO << ">>> WARNING: no traffic light found in the network. \n" << std::flush;

        // for each traffic light
        for (auto &TLid : TLList)
        {
            // get all incoming lanes
            auto lan = TraCI->TLGetControlledLanes(TLid);

            // remove duplicate entries
            sort( lan.begin(), lan.end() );
            lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

            // for each incoming lane
            for(auto &it2 : lan)
                incomingLanes[it2] = TLid;
        }
    }
}


void IntersectionDelay::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(record_intersectionDelay_stat)
    {
        // make sure there is at least one TL in the network
        if(!TLList.empty())
            vehiclesDelay();
    }
}


void IntersectionDelay::vehiclesDelay()
{
    // iterate over all vehicles in the network
    for(auto vID : TraCI->vehicleGetIDList())
    {
        // look for the vehicle
        auto loc = vehDelay_perTL.find(vID);

        if(loc == vehDelay_perTL.end())
        {
            auto TL = TraCI->vehicleGetNextTLS(vID);

            // get the id of the next TL
            std::string TLid = "";
            if(!TL.empty())
                TLid = TL[0].TLS_id;

            // there is no TL ahead of this vehicle
            if(TLid == "")
                continue;

            delayEntry_t *entry = generateEmptyDelay(vID, TLid);
            std::vector<delayEntry_t *> thisIntersection = {entry};

            auto r = vehDelay_perTL.insert(std::make_pair(vID,thisIntersection));

            vehiclesDelayStart(vID, r.first->second.back());
            vehiclesDelayDuration(vID, r.first->second.back());
        }
        else
        {
            // if the vehicle crossed the intersection
            if(loc->second.back()->crossed)
            {
                // keep checking the next TL along the way
                // and add it if a new TL appears (ex. re-routing)
                auto TL = TraCI->vehicleGetNextTLS(vID);

                // get the id of the next TL
                std::string TLid = "";
                if(!TL.empty())
                    TLid = TL[0].TLS_id;

                // check if TLid has changed
                if(TLid != "" && loc->second.back()->TLid != TLid)
                {
                    delayEntry_t *entry = generateEmptyDelay(vID, TLid);
                    loc->second.push_back(entry);
                }
            }

            for(int index = loc->second.size()-1; index >= 0; index--)
            {
                // keep measuring delay as long as endDelay is unknown!
                if(loc->second[index]->endDelay == -1)
                {
                    vehiclesDelayStart(vID, loc->second[index]);
                    vehiclesDelayDuration(vID, loc->second[index]);
                }
            }
        }
    }
}


void IntersectionDelay::vehiclesDelayStart(std::string vID, delayEntry_t *vehDelay)
{
    // wait for the vehicle to enter into an incoming lane
    if(!vehDelay->onIncomingLane)
    {
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // If not on one of the incoming lanes of this TL
        auto itl = incomingLanes.find(currentLane);
        if(itl == incomingLanes.end() || itl->second != vehDelay->TLid)
            return;

        vehDelay->onIncomingLane = true;
    }

    // if the veh is on its last lane before the intersection
    if(vehDelay->lastLane == "")
    {
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // get best lanes for this vehicle
        auto best = TraCI->vehicleGetBestLanes(vID);

        bool found = false;
        for(auto &y : best)
        {
            std::string laneID = y.second.laneId;
            int offset = y.second.offset;
            int continuingDrive = y.second.continuingDrive;

            if(offset == 0 && continuingDrive == 1 && laneID == currentLane)
            {
                vehDelay->lastLane = laneID;
                vehDelay->intersectionEntrance = omnetpp::simTime().dbl();

                if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
                {
                    printf("*** %-10s is approaching TL %s on lane %s \n", vID.c_str(), vehDelay->TLid.c_str(), laneID.c_str());
                    std::cout.flush();
                }

                found = true;
                break;
            }
        }

        if(!found)
            return;
    }

    // if we have not crossed the intersection yet
    if(!vehDelay->crossed)
    {
        // get current lane
        std::string currentLane = TraCI->vehicleGetLaneID(vID);

        // If we are at the middle of intersection
        if(incomingLanes.find(currentLane) == incomingLanes.end())
        {
            vehDelay->crossed = true;
            vehDelay->crossedTime = omnetpp::simTime().dbl();

            if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
            {
                printf("*** %-10s crossed TL %s on lane %s \n", vID.c_str(), vehDelay->TLid.c_str(), vehDelay->lastLane.c_str());
                std::cout.flush();
            }
        }
    }

    // looking for the start of deceleration delay
    if(vehDelay->startDeccel == -1 && !vehDelay->crossed)
    {
        // get speed
        double speed = TraCI->vehicleGetSpeed(vID);
        vehDelay->lastSpeeds.push_back( std::make_pair(omnetpp::simTime().dbl(), speed) );

        // get acceleration
        double accel = TraCI->vehicleGetCurrentAccel(vID);
        vehDelay->lastAccels.push_back( std::make_pair(omnetpp::simTime().dbl(), accel) );

        // get next TL link state
        std::vector<TL_info_t> res = TraCI->vehicleGetNextTLS(vID);
        if(res.empty())
            throw omnetpp::cRuntimeError("there is no next TL for vehicle '%s'", vID.c_str());
        vehDelay->lastSignals.push_back(res[0].linkState);

        // wait for buffers to become full
        if(!vehDelay->lastAccels.full())
            return;

        for(auto &g : vehDelay->lastAccels)
            if( g.second > deccelDelayThreshold ) return;

        // At this point a slow down is found.
        // We should check if this slow down was due to a Y or R signal
        bool YorR = false;
        for(auto &g : vehDelay->lastSignals)
        {
            if(g == 'y' || g == 'r')
            {
                YorR = true;
                break;
            }
        }

        if(!YorR)
            return;

        vehDelay->startDeccel = vehDelay->lastAccels[0].first;
        vehDelay->lastAccels.clear();

        vehDelay->oldSpeed = vehDelay->lastSpeeds[0].second;
        vehDelay->lastSpeeds.clear();

        vehDelay->lastSignals.clear();

        if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
        {
            printf("*** %-10s is decelerating on lane %s \n", vID.c_str(), vehDelay->lastLane.c_str());
            std::cout.flush();
        }
    }

    // looking for the start of stopping delay
    // NOTE: stopping delay might be zero
    if(vehDelay->startDeccel != -1 && vehDelay->startStopping == -1 && !vehDelay->crossed)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        vehDelay->lastSpeeds.push_back( std::make_pair(omnetpp::simTime().dbl(), speed) );

        if(vehDelay->lastSpeeds.full())
        {
            bool isStopped = true;
            for(auto& g : vehDelay->lastSpeeds)
            {
                if( g.second > vehDelay->stoppingDelayThreshold )
                {
                    isStopped = false;
                    break;
                }
            }

            if(isStopped)
            {
                vehDelay->startStopping = vehDelay->lastSpeeds[0].first;
                vehDelay->lastSpeeds.clear();

                if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
                {
                    printf("*** %-10s is stopping on lane %s. \n", vID.c_str(), vehDelay->lastLane.c_str());
                    std::cout.flush();
                }
            }
        }
    }

    // looking for the start of accel delay
    if(vehDelay->startDeccel != -1 && vehDelay->startAccel == -1 && vehDelay->crossed)
    {
        double speed = TraCI->vehicleGetSpeed(vID);
        vehDelay->lastSpeeds2.push_back( std::make_pair(omnetpp::simTime().dbl(), speed) );

        if(vehDelay->lastSpeeds2.full())
        {
            for(auto &g : vehDelay->lastSpeeds2)
                if( g.second < vehDelay->stoppingDelayThreshold ) return;

            vehDelay->startAccel = vehDelay->lastSpeeds2[0].first;
            vehDelay->lastSpeeds2.clear();

            if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 2)
            {
                printf("*** %-10s is accelerating on lane %s. \n", vID.c_str(), vehDelay->lastLane.c_str());
                std::cout.flush();
            }
        }
        else return;
    }

    // looking for the end of delay
    if(vehDelay->startDeccel != -1 && vehDelay->startAccel != -1 && vehDelay->endDelay == -1 && vehDelay->crossed)
    {
        if(TraCI->vehicleGetSpeed(vID) >= vehDelay->oldSpeed)
            vehDelay->endDelay = omnetpp::simTime().dbl();
        else return;
    }
}


void IntersectionDelay::vehiclesDelayDuration(std::string vID, delayEntry_t *vehDelay)
{
    if(vehDelay->startDeccel == -1)
        return;

    // update deceleration delay duration
    if(vehDelay->startStopping != -1)
        vehDelay->decelDelay = vehDelay->startStopping - vehDelay->startDeccel;
    else if(vehDelay->startAccel != -1)
        vehDelay->decelDelay = vehDelay->startAccel - vehDelay->startDeccel;
    else
        vehDelay->decelDelay = omnetpp::simTime().dbl() - vehDelay->startDeccel;

    if(vehDelay->decelDelay < 0 || vehDelay->decelDelay > omnetpp::simTime().dbl())
        throw omnetpp::cRuntimeError("deceleration delay value is incorrect for vehicle %s", vID.c_str());

    // update waiting delay duration
    if(vehDelay->startStopping != -1)
    {
        if(vehDelay->startAccel != -1)
            vehDelay->waitingDelay = vehDelay->startAccel - vehDelay->startStopping;
        else
            vehDelay->waitingDelay = omnetpp::simTime().dbl() - vehDelay->startStopping;

        if(vehDelay->waitingDelay < 0 || vehDelay->waitingDelay > omnetpp::simTime().dbl())
            throw omnetpp::cRuntimeError("waiting delay value is incorrect for vehicle %s", vID.c_str());
    }

    // update accelDelay
    if(vehDelay->startAccel != -1)
    {
        if(vehDelay->endDelay != -1)
            vehDelay->accelDelay = vehDelay->endDelay - vehDelay->startAccel;
        else
            vehDelay->accelDelay = omnetpp::simTime().dbl() - vehDelay->startAccel;
    }

    // update total delay duration
    vehDelay->totalDelay = omnetpp::simTime().dbl() - vehDelay->startDeccel;

    if(vehDelay->totalDelay < 0 || vehDelay->totalDelay > omnetpp::simTime().dbl())
        throw omnetpp::cRuntimeError("accumulated delay value is incorrect for vehicle %s", vID.c_str());
}


delayEntry_t* IntersectionDelay::generateEmptyDelay(std::string vID, std::string TLid)
{
    std::string vehType = TraCI->vehicleGetTypeID(vID);

    double stoppingDelayThreshold = 0;
    if(vehType == "bicycle")
        stoppingDelayThreshold = speedThreshold_bike;
    else
        stoppingDelayThreshold = speedThreshold_veh;

    delayEntry_t *entry = new delayEntry_t();

    entry->TLid = TLid;
    entry->vehType = vehType;
    entry->onIncomingLane = false;
    entry->lastLane = "";
    entry->intersectionEntrance = -1;
    entry->crossed = false;
    entry->crossedTime = -1;
    entry->oldSpeed = -1;
    entry->stoppingDelayThreshold = stoppingDelayThreshold;
    entry->startDeccel = -1;
    entry->startStopping = -1;
    entry->startAccel = -1;
    entry->endDelay = -1;
    entry->decelDelay = 0;
    entry->waitingDelay = 0;
    entry->totalDelay = 0;

    // get simulation time step in seconds
    double TS = (double)TraCI->simulationGetDelta() / 1000.;

    // how much further we need to look at speed/accel/signal data
    int buffSize = std::floor(1. / TS);

    // buffer size cannot be lower than 1
    buffSize = std::max(1, buffSize);

    boost::circular_buffer<std::pair<double,double>> CB_speed;  // create a circular buffer
    CB_speed.set_capacity(buffSize);                            // set max capacity
    CB_speed.clear();
    entry->lastSpeeds = CB_speed;

    boost::circular_buffer<std::pair<double,double>> CB_speed2;  // create a circular buffer
    CB_speed2.set_capacity(buffSize);                            // set max capacity
    CB_speed2.clear();
    entry->lastSpeeds2 = CB_speed2;

    boost::circular_buffer<std::pair<double,double>> CB_accel;  // create a circular buffer
    CB_accel.set_capacity(buffSize);                            // set max capacity
    CB_accel.clear();
    entry->lastAccels = CB_accel;

    boost::circular_buffer<char> CB_sig;   // create a circular buffer
    CB_sig.set_capacity(buffSize);         // set max capacity
    CB_sig.clear();
    entry->lastSignals = CB_sig;

    return entry;
}


delayEntry_t* IntersectionDelay::vehicleGetDelay(std::string vID, std::string TLid)
{
    auto it = vehDelay_perTL.find(vID);
    if(it == vehDelay_perTL.end())
        return NULL;

    // delay in each intersection that this vehicle has encountered so far
    auto allDelays = it->second;

    // iterate from the last visited intersection
    for(auto itt = allDelays.rbegin(); itt != allDelays.rend(); ++itt)
    {
        if((*itt)->TLid == TLid)
            return *itt;
    }

    return NULL;
}


void IntersectionDelay::vehiclesDelayToFile()
{
    if(vehDelay_perTL.empty())
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

        // get configuration name
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetDelta());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime_str().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime_str().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration_str().c_str());
    }

    // write header
    fprintf (filePtr, "%-25s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleType");
    fprintf (filePtr, "%-50s","TLid");
    fprintf (filePtr, "%-15s","crossedT");
    fprintf (filePtr, "%-15s","VbeforeDeccel");
    fprintf (filePtr, "%-15s","startDeccel");
    fprintf (filePtr, "%-15s","startStopping");
    fprintf (filePtr, "%-15s","startAccel");
    fprintf (filePtr, "%-15s","endDelay");
    fprintf (filePtr, "%-17s","decelDuration");
    fprintf (filePtr, "%-17s","stoppingDuration");
    fprintf (filePtr, "%-17s","accelDuration");
    fprintf (filePtr, "%-17s \n\n","totalDelay");

    // write body
    for(auto &y : vehDelay_perTL)
    {
        for(auto &z : y.second)
        {
            fprintf (filePtr, "%-25s", y.first.c_str());
            fprintf (filePtr, "%-20s", z->vehType.c_str());
            fprintf (filePtr, "%-50s", z->TLid.c_str());
            fprintf (filePtr, "%-15.2f", z->crossedTime);
            fprintf (filePtr, "%-15.2f", z->oldSpeed);
            fprintf (filePtr, "%-15.2f", z->startDeccel);
            fprintf (filePtr, "%-15.2f", z->startStopping);
            fprintf (filePtr, "%-15.2f", z->startAccel);
            fprintf (filePtr, "%-15.2f", z->endDelay);
            fprintf (filePtr, "%-17.2f", z->decelDelay);
            fprintf (filePtr, "%-17.2f", z->waitingDelay);
            fprintf (filePtr, "%-17.2f", z->accelDelay);
            fprintf (filePtr, "%-17.2f\n", z->totalDelay);
        }

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}

}
