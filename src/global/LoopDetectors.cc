/****************************************************************************/
/// @file    LoopDetectors.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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
#undef ev
#include "boost/filesystem.hpp"

#include "global/LoopDetectors.h"
#include "logging/VENTOS_logging.h"

namespace VENTOS {

Define_Module(VENTOS::LoopDetectors);


LoopDetectors::~LoopDetectors()
{

}


void LoopDetectors::initialize(int stage)
{
    if(stage == 0)
    {
        record_stat = par("record_stat").boolValue();

        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        Signal_initialize_withTraCI = registerSignal("initializeWithTraCISignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initializeWithTraCISignal", this);

        Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTimeStepSignal", this);
    }
}


void LoopDetectors::finish()
{
    saveLDsData();

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initializeWithTraCISignal", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTimeStepSignal", this);
}


void LoopDetectors::handleMessage(omnetpp::cMessage *msg)
{

}


void LoopDetectors::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep();
    }
}


void LoopDetectors::initialize_withTraCI()
{
    if(record_stat)
        checkLoopDetectors();
}


void LoopDetectors::executeEachTimeStep()
{
    if(record_stat)
        collectLDsData();
}


void LoopDetectors::checkLoopDetectors()
{
    // get all loop detectors
    auto str = TraCI->LDGetIDList();

    if(str.empty())
        LOG_INFO << ">>> WARNING: no loop detectors found in the network. \n" << std::flush;

    // for each loop detector
    for (auto &it : str)
    {
        std::string lane = TraCI->LDGetLaneID(it);
        LDs[it] = lane;
    }

    if(str.size() > 0)
        LOG_INFO << boost::format(">>> %1% loop detectors are present in the network. \n") % str.size() << std::flush;
}


void LoopDetectors::collectLDsData()
{
    // for each loop detector
    for (auto &entry : LDs)
    {
        std::string detectorName = entry.first;
        std::string detectorLane = entry.second;

        auto st = TraCI->LDGetLastStepVehicleData(detectorName);

        // proceed only if this loop detector detected a vehicle
        if(st.size() == 0)
            continue;

        // get vehicle information
        std::string vehicleName = st[0].vehID;
        double entryT = st[0].entryTime;
        double leaveT = st[0].leaveTime;
        double speed = TraCI->LDGetLastStepMeanVehicleSpeed(detectorName);  // vehicle speed at current moment

        auto counter = std::find_if(Vec_loopDetectors.begin(), Vec_loopDetectors.end(), [detectorName, vehicleName](LoopDetectorData_t const& n)
                {return (n.detectorName == detectorName && n.vehicleName == vehicleName);});

        // its a new entry, so we add detectorName
        if(counter == Vec_loopDetectors.end())
        {
            LoopDetectorData_t tmp = {};

            tmp.detectorName = detectorName;
            tmp.lane = detectorLane;
            tmp.vehicleName = vehicleName;
            tmp.entryTime = entryT;
            tmp.entrySpeed = speed;
            tmp.leaveTime = leaveT;
            tmp.leaveSpeed = speed;

            Vec_loopDetectors.push_back(tmp);
        }
        // if found, just update leaveTime and leaveSpeed
        else
        {
            counter->leaveTime = leaveT;
            counter->leaveSpeed = speed;
        }
    }
}


void LoopDetectors::saveLDsData()
{
    if(Vec_loopDetectors.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_loopDetector.txt") % currentRun;

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
    fprintf (filePtr, "%-20s","loopDetector");
    fprintf (filePtr, "%-15s","lane");
    fprintf (filePtr, "%-22s","vehicleName");
    fprintf (filePtr, "%-20s","entryTime");
    fprintf (filePtr, "%-22s","entrySpeed");
    fprintf (filePtr, "%-20s","leaveTime");
    fprintf (filePtr, "%-22s\n\n","leaveSpeed");

    // write body
    for(auto &y : Vec_loopDetectors)
    {
        fprintf (filePtr, "%-20s", y.detectorName.c_str());
        fprintf (filePtr, "%-15s", y.lane.c_str());
        fprintf (filePtr, "%-22s", y.vehicleName.c_str());
        fprintf (filePtr, "%-20.2f", y.entryTime);
        fprintf (filePtr, "%-22.2f", y.entrySpeed);
        fprintf (filePtr, "%-20.2f", y.leaveTime);
        fprintf (filePtr, "%-22.2f\n", y.leaveSpeed);
    }

    fclose(filePtr);
}

}
