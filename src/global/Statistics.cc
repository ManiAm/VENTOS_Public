/****************************************************************************/
/// @file    Statistics.cc
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

/*
 * This class collects simulation data from entities that are created/removed dynamically
 * in the simulation (like vehicles and bikes).
 * */

#undef ev
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "global/Statistics.h"

namespace VENTOS {


Define_Module(VENTOS::Statistics);

Statistics::~Statistics()
{

}


void Statistics::initialize(int stage)
{
    if(stage == 0)
    {
        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        record_sim_stat = par("record_sim_stat").boolValue();

        Signal_initialize_withTraCI = registerSignal("initializeWithTraCISignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initializeWithTraCISignal", this);

        Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTimeStepSignal", this);

        Signal_module_added = registerSignal("vehicleModuleAddedSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("vehicleModuleAddedSignal", this);

        Signal_arrived = registerSignal("vehicleArrivedSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("vehicleArrivedSignal", this);
    }
}


void Statistics::finish()
{
    save_beacon_stat_toFile();

    save_plnData_toFile();
    save_plnConfig_toFile();

    save_MAC_stat_toFile();
    save_PHY_stat_toFile();
    save_FrameTxRx_stat_toFile();

    // record simulation data one last time before closing TraCI
    if(!TraCI->TraCIclosedOnError)
        record_Sim_data();

    save_Sim_data_toFile();
    save_Veh_data_toFile();
    save_Veh_emission_toFile();

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initializeWithTraCISignal", this);
}


void Statistics::handleMessage(omnetpp::cMessage *msg)
{

}


void Statistics::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        updateInterval = (double)TraCI->simulationGetDelta() / 1000.;

        init_Sim_data();
    }
    else if(signalID == Signal_executeEachTS)
    {
        // record simulation data after proceeding one time step
        record_Sim_data();

        // collecting data for this vehicle in this timeStep
        for(auto &module : TraCI->hosts)
        {
            record_Veh_data(module.first);
            record_Veh_emission(module.first);
        }
    }
}


void Statistics::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, const char *SUMOID, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_arrived)
    {
        // update vehicle status one last time
        record_Veh_data(SUMOID, true);
    }
}


void Statistics::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    // note that this signal can be emitted more than once for a vehicle
    // when that vehicle enters/exits ROI (region of interest)
    if(signalID == Signal_module_added)
    {
        omnetpp::cModule *m = static_cast<omnetpp::cModule *>(obj);
        ASSERT(m);

        if(m->hasPar("record_stat") && m->par("record_stat").boolValue())
        {
            std::string vClass = m->par("vehicleClass");

            if(vClass == "pedestrian")
                init_Ped_data(m->par("SUMOID").stringValue(), m);
            else // obstacle, bikes, cars, trucks, etc.
                init_Veh_data(m->par("SUMOID").stringValue(), m);
        }

        if(m->hasPar("record_emission") && m->par("record_emission").boolValue())
        {
            std::string vClass = m->par("vehicleClass");

            if(vClass != "pedestrian" && vClass != "custom1")
                init_Veh_emission(m->par("SUMOID").stringValue(), m);
        }
    }
}


void Statistics::save_beacon_stat_toFile()
{
    if(global_Beacon_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_beaconsStat.txt") % currentRun;

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
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from");
    fprintf (filePtr, "%-20s","to");
    fprintf (filePtr, "%-20s\n\n","dropped");

    for(auto &y : global_Beacon_stat)
    {
        fprintf (filePtr, "%-12.2f", y.time);
        fprintf (filePtr, "%-20s", y.senderID.c_str());
        fprintf (filePtr, "%-20s", y.receiverID.c_str());
        fprintf (filePtr, "%-20d \n", y.dropped);
    }

    fclose(filePtr);
}


void Statistics::save_plnConfig_toFile()
{
    if(global_plnConfig_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_plnConfig.txt") % currentRun;

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

    // write column title
    fprintf (filePtr, "%-15s","index");
    fprintf (filePtr, "%-15s","timestamp");
    fprintf (filePtr, "%-15s","vehId");
    fprintf (filePtr, "%-15s","pltMode");
    fprintf (filePtr, "%-15s","pltId");
    fprintf (filePtr, "%-15s","pltDepth");
    fprintf (filePtr, "%-15s","pltSize");
    fprintf (filePtr, "%-15s","pltOptSize");
    fprintf (filePtr, "%-15s","pltMaxSize");
    fprintf (filePtr, "\n\n");

    // sort the global_plnConfig_stat
    std::sort(global_plnConfig_stat.begin(), global_plnConfig_stat.end(),
            [](const plnConfig_t & a, const plnConfig_t & b) -> bool
            {
        if(a.timestamp < b.timestamp)
            return true;
        else if((a.timestamp == b.timestamp) && (a.pltId < b.pltId))
            return true;
        else if((a.timestamp == b.timestamp) && (a.pltId == b.pltId) && (a.pltDepth < b.pltDepth))
            return true;
        else
            return false;
            });

    double oldTime = -2;
    std::string oldPltId = "";
    int index = 0;

    for(auto &y : global_plnConfig_stat)
    {
        if(oldTime != y.timestamp || oldPltId != y.pltId)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timestamp;
            oldPltId = y.pltId;
            index++;
        }

        fprintf (filePtr, "%-15d", index);
        fprintf (filePtr, "%-15.2f", y.timestamp);
        fprintf (filePtr, "%-15s", y.vehId.c_str());
        fprintf (filePtr, "%-15d", y.pltMode);
        fprintf (filePtr, "%-15s", y.pltId.c_str());
        fprintf (filePtr, "%-15d", y.pltDepth);

        if(y.pltSize != -1)
            fprintf (filePtr, "%-15d", y.pltSize);
        else
            fprintf (filePtr, "%-15s", "-");

        if(y.optSize != -1)
            fprintf (filePtr, "%-15d", y.optSize);
        else
            fprintf (filePtr, "%-15s", "-");

        if(y.maxSize != -1)
            fprintf (filePtr, "%-15d", y.maxSize);
        else
            fprintf (filePtr, "%-15s", "-");

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}


void Statistics::save_plnData_toFile()
{
    if(global_plnStateChange_stat.empty() || global_plnDataExchange_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_plnData.txt") % currentRun;

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

    std::vector<double> timeSteps;

    for(auto &i : global_plnStateChange_stat)
    {
        auto iter = std::find(timeSteps.begin(), timeSteps.end(), i.time);
        if(iter == timeSteps.end())
            timeSteps.push_back(i.time);
    }

    for(auto &i : global_plnDataExchange_stat)
    {
        auto iter = std::find(timeSteps.begin(), timeSteps.end(), i.time);
        if(iter == timeSteps.end())
            timeSteps.push_back(i.time);
    }

    // sort time values in timeSteps
    std::sort(timeSteps.begin(), timeSteps.end());

    typedef struct platoonDataAll
    {
        double time = -1;
        std::string vehId = "-";
        std::string commandSent = "-";
        std::string receiverId = "-";
        std::string senderPltId = "-";
        std::string receiverPltId = "-";
        std::string fromState = "-";
        std::string toState = "-";
        std::string maneuverStartEnd = "-";
    } platoonDataAll_t;

    std::vector<platoonDataAll_t> platoonData_vector;

    // iterate over each time step that a platooning event has happened
    for(auto &time : timeSteps)
    {
        // get all matches in 'global_plnStateChange_stat' for this time step
        std::vector<std::vector<plnStateChange_t>::const_iterator> matches_plnState;
        auto i = global_plnStateChange_stat.begin(), end = global_plnStateChange_stat.end();
        while (i != end)
        {
            i = std::find_if(i, end, [&] (const plnStateChange_t &s) {return s.time == time;});
            if (i != end)
                matches_plnState.push_back(i++);
        }

        // get all matches in 'global_plnDataExchange_stat' fro this time step
        typedef std::vector<std::vector<plnDataExchange_t>::const_iterator> matches_plnData_ii;
        matches_plnData_ii matches_plnData;
        auto ii = global_plnDataExchange_stat.begin(), end2 = global_plnDataExchange_stat.end();
        while (ii != end2)
        {
            ii = std::find_if(ii, end2, [&] (const plnDataExchange_t &s) {return s.time == time;});
            if (ii != end2)
                matches_plnData.push_back(ii++);
        }

        if(!matches_plnState.empty())
        {
            // iterate over all platoon states in this time step
            for(auto &plnState : matches_plnState)
            {
                platoonDataAll_t entry;

                entry.time = time;
                entry.vehId = plnState->vehId;
                entry.fromState = plnState->fromState;
                entry.toState = plnState->toState;

                // does the vehicle send any command in this state
                std::vector<matches_plnData_ii::iterator> matches_commands;
                auto ii = matches_plnData.begin(), end = matches_plnData.end();
                while (ii != end)
                {
                    ii = std::find_if(ii, end, [&] (const std::vector<plnDataExchange_t>::const_iterator &s) {
                        return (s->senderId == plnState->vehId && s->senderState == plnState->fromState);});
                    if (ii != end)
                        matches_commands.push_back(ii++);
                }

                if(matches_commands.empty())
                {
                    platoonData_vector.push_back(entry);
                }
                else
                {
                    // for each command
                    for(auto &k : matches_commands)
                    {
                        entry.commandSent = (*k)->command;
                        entry.receiverId = (*k)->receiverId;
                        entry.senderPltId = (*k)->sendingPltId;
                        entry.receiverPltId = (*k)->receivingPltId;

                        platoonData_vector.push_back(entry);
                    }
                }
            }
        }
        else
        {
            if(matches_plnData.empty())
                throw omnetpp::cRuntimeError("matches_plnState and matches_plnData are both empty in time step '%f'", time);

            // iterate over all data exchange in this time step
            for(auto &plnData : matches_plnData)
            {
                platoonDataAll_t entry;

                entry.time = time;
                entry.vehId = plnData->senderId;
                entry.fromState = plnData->senderState;
                entry.toState = plnData->senderState;  // the vehicle state does not change
                entry.commandSent = plnData->command;
                entry.receiverId = plnData->receiverId;
                entry.senderPltId = plnData->sendingPltId;
                entry.receiverPltId = plnData->receivingPltId;

                platoonData_vector.push_back(entry);
            }
        }
    }

    // mark start/end of maneuvers
    for(auto &entry : global_plnManeuverDuration_stat)
    {
        auto ii = std::find_if(platoonData_vector.begin(), platoonData_vector.end(), [&] (const platoonDataAll_t &s) {
            return (s.time == entry.time && s.vehId == entry.vehId && s.toState == entry.vehState);});

        if(ii != platoonData_vector.end())
            ii->maneuverStartEnd = entry.maneuver;
        else
            throw omnetpp::cRuntimeError("cannot find the maneuver entry in platoonData_vector map");
    }

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-15s","vehId");
    fprintf (filePtr, "%-30s","fromState");
    fprintf (filePtr, "%-30s","toState");
    fprintf (filePtr, "%-15s","commandSent");
    fprintf (filePtr, "%-15s","receiverId");
    fprintf (filePtr, "%-15s","senderPltId");
    fprintf (filePtr, "%-17s","receiverPltId");
    fprintf (filePtr, "%-17s\n\n","maneuverStartEnd");

    std::string oldSender = "";
    double oldTime = -1;

    // write body
    for(auto &y : platoonData_vector)
    {
        // make the log more readable :)
        if(y.vehId != oldSender || y.time != oldTime)
        {
            fprintf(filePtr, "\n");
            oldSender = y.vehId;
            oldTime = y.time;
        }

        fprintf (filePtr, "%-12.2f", y.time);
        fprintf (filePtr, "%-15s", y.vehId.c_str());
        fprintf (filePtr, "%-30s", y.fromState.c_str());
        fprintf (filePtr, "%-30s", y.toState.c_str());
        fprintf (filePtr, "%-15s", y.commandSent.c_str());
        fprintf (filePtr, "%-15s", y.receiverId.c_str());
        fprintf (filePtr, "%-15s", y.senderPltId.c_str());
        fprintf (filePtr, "%-17s", y.receiverPltId.c_str());
        fprintf (filePtr, "%-17s\n", y.maneuverStartEnd.c_str());
    }

    fclose(filePtr);
}


void Statistics::save_MAC_stat_toFile()
{
    if(global_MAC_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_MACdata.txt") % currentRun;

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
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","lastStatTime");
    fprintf (filePtr, "%-20s","NumDroppedFrames");
    fprintf (filePtr, "%-20s","NumTooLittleTime");
    fprintf (filePtr, "%-30s","NumInternalContention");
    fprintf (filePtr, "%-20s","NumBackoff");
    fprintf (filePtr, "%-20s","SlotsBackoff");
    fprintf (filePtr, "%-20s \n\n","TotalBusyTime");

    // write body
    for(auto &y : global_MAC_stat)
    {
        fprintf (filePtr, "%-20s", y.first.c_str());
        fprintf (filePtr, "%-20.8f", y.second.last_stat_time);
        fprintf (filePtr, "%-20ld", y.second.NumDroppedFrames);
        fprintf (filePtr, "%-20ld", y.second.NumTooLittleTime);
        fprintf (filePtr, "%-30ld", y.second.NumInternalContention);
        fprintf (filePtr, "%-20ld", y.second.NumBackoff);
        fprintf (filePtr, "%-20ld", y.second.SlotsBackoff);
        fprintf (filePtr, "%-20.8f \n", y.second.TotalBusyTime);
    }

    fclose(filePtr);
}


void Statistics::save_PHY_stat_toFile()
{
    if(global_PHY_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_PHYdata.txt") % currentRun;

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
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","lastStatTime");
    fprintf (filePtr, "%-15s","NumSentFrames");
    fprintf (filePtr, "%-20s","NumReceivedFrames");
    fprintf (filePtr, "%-25s","NumLostFrames_BiteError");
    fprintf (filePtr, "%-25s","NumLostFrames_Collision");
    fprintf (filePtr, "%-25s\n\n","NumLostFrames_TXRX");

    // write body
    for(auto &y : global_PHY_stat)
    {
        fprintf (filePtr, "%-20s", y.first.c_str());
        fprintf (filePtr, "%-20.8f", y.second.last_stat_time);
        fprintf (filePtr, "%-15ld", y.second.NumSentFrames);
        fprintf (filePtr, "%-20ld", y.second.NumReceivedFrames);
        fprintf (filePtr, "%-25ld", y.second.NumLostFrames_BiteError);
        fprintf (filePtr, "%-25ld", y.second.NumLostFrames_Collision);
        fprintf (filePtr, "%-25ld\n", y.second.NumLostFrames_TXRX);
    }

    fclose(filePtr);
}


void Statistics::save_FrameTxRx_stat_toFile()
{
    if(global_frameTxRx_stat.empty())
        return;

    typedef struct msgTxRxStat_vec
    {
        msgTxRxStat_t entry;
        long int MsgID;
        long int nic;
    } msgTxRxStat_vec_t;

    // copy the map into a vector
    std::vector<msgTxRxStat_vec_t> global_frameTxRx_stat_vec;
    for(auto &y : global_frameTxRx_stat)
    {
        msgTxRxStat_vec_t newEntry;

        newEntry.entry = y.second;
        newEntry.MsgID = y.first.first;
        newEntry.nic = y.first.second;

        global_frameTxRx_stat_vec.push_back(newEntry);
    }

    // sort the vector
    std::sort(global_frameTxRx_stat_vec.begin(), global_frameTxRx_stat_vec.end(),
            [](const msgTxRxStat_vec_t &a, const msgTxRxStat_vec_t &b) -> bool {
        if(a.entry.SentAt < b.entry.SentAt)
            return true;
        else if(a.entry.SentAt == b.entry.SentAt && a.entry.SenderNode < b.entry.SenderNode)
            return true;
        else if(a.entry.SentAt == b.entry.SentAt && a.entry.SenderNode == b.entry.SenderNode && a.entry.DistanceToReceiver < b.entry.DistanceToReceiver)
            return true;
        else
            return false;
    });

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_FrameTxRxdata.txt") % currentRun;

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
    fprintf (filePtr, "%-20s","MsgId");
    fprintf (filePtr, "%-20s","MsgName");
    fprintf (filePtr, "%-20s","SenderNode");
    fprintf (filePtr, "%-20s","ReceiverNode");
    fprintf (filePtr, "%-20s","ReceiverGateId");
    fprintf (filePtr, "%-20s","SendingStartAt");
    fprintf (filePtr, "%-20s","FrameSize");
    fprintf (filePtr, "%-20s","ChannelNum");
    fprintf (filePtr, "%-20s","TransmissionPower");
    fprintf (filePtr, "%-20s","TransmissionSpeed");
    fprintf (filePtr, "%-20s","TransmissionTime");
    fprintf (filePtr, "%-20s","DistanceToReceiver");
    fprintf (filePtr, "%-22s","PropagationDelay");
    fprintf (filePtr, "%-20s","ReceptionEndAt");
    fprintf (filePtr, "%-20s\n\n","FrameRxStatus");

    // write body
    std::string oldSender = "";
    for(auto &y : global_frameTxRx_stat_vec)
    {
        if(oldSender != y.entry.SenderNode)
        {
            fprintf(filePtr, "\n");
            oldSender = y.entry.SenderNode;
        }

        fprintf (filePtr, "%-20ld", y.MsgID);
        fprintf (filePtr, "%-20s", y.entry.MsgName.c_str());
        fprintf (filePtr, "%-20s", y.entry.SenderNode.c_str());
        fprintf (filePtr, "%-20s", y.entry.ReceiverNode.c_str());

        if(y.nic != -1)
            fprintf (filePtr, "%-20ld", y.nic);
        else
            fprintf (filePtr, "%-20s", "-");

        fprintf (filePtr, "%-20.8f", y.entry.SentAt);
        fprintf (filePtr, "%-20d", y.entry.FrameSize);

        if(y.entry.channelNum != -1)
            fprintf (filePtr, "%-20d", y.entry.channelNum);
        else
            fprintf (filePtr, "%-20s", "-");

        fprintf (filePtr, "%-20.2f", y.entry.TransmissionPower);  // it is already in mW
        fprintf (filePtr, "%-20.2f", y.entry.TransmissionSpeed / 1000000.);  // change unit to Mbps
        fprintf (filePtr, "%-20.8f", y.entry.TransmissionTime);

        if(y.entry.DistanceToReceiver != -1)
            fprintf (filePtr, "%-20.8f", y.entry.DistanceToReceiver);
        else
            fprintf (filePtr, "%-20.8s", "-");

        if(y.entry.PropagationDelay != -1)
            fprintf (filePtr, "%-22.13f", y.entry.PropagationDelay);
        else
            fprintf (filePtr, "%-22.13s", "-");

        if(y.entry.ReceivedAt != -1)
            fprintf (filePtr, "%-20.8f", y.entry.ReceivedAt);
        else
            fprintf (filePtr, "%-20.8s", "-");

        fprintf (filePtr, "%-20s\n", y.entry.FrameRxStatus.c_str());
    }

    fclose(filePtr);
}


void Statistics::init_Sim_data()
{
    if(!record_sim_stat)
        return;

    std::string record_sim_list = par("record_sim_list").stringValue();
    // make sure record_sim_list is not empty
    if(record_sim_list == "")
        throw omnetpp::cRuntimeError("record_sim_list is empty");

    // applications are separated by |
    std::vector<std::string> records;
    boost::split(records, record_sim_list, boost::is_any_of("|"));

    // iterate over each application
    for(std::string appl : records)
    {
        if(appl == "")
            throw omnetpp::cRuntimeError("Invalid record_sim_list format");

        // remove leading and trailing spaces from the string
        boost::trim(appl);

        // convert to lower case
        boost::algorithm::to_lower(appl);

        record_sim_tokenize.push_back(appl);
    }

    // record simulation data right after TraCI establishment
    record_Sim_data();
}


void Statistics::record_Sim_data()
{
    if(!record_sim_stat)
        return;

    sim_status_entry_t entry = {};

    entry.timeStep = (omnetpp::simTime()-updateInterval).dbl();

    for(std::string record : record_sim_tokenize)
    {
        if(record == "loaded")
            entry.loaded = TraCI->simulationGetLoadedVehiclesCount();
        else if(record == "departed")
            entry.departed = departedVehicleCount;
        else if(record == "arrived")
            entry.arrived = arrivedVehicleCount;
        else if(record == "running")
            entry.running = activeVehicleCount;
        else if(record == "waiting")
            entry.waiting = TraCI->simulationGetMinExpectedNumber() - activeVehicleCount;
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name", record.c_str());
    }

    sim_record_status.push_back(entry);
}


void Statistics::save_Sim_data_toFile()
{
    if(sim_record_status.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_simData.txt") % currentRun;

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

    // write column title
    fprintf (filePtr, "%-15s","index");
    fprintf (filePtr, "%-15s","timeStep");
    for(std::string column : record_sim_tokenize)
        fprintf (filePtr, "%-15s", column.c_str());
    fprintf (filePtr, "\n\n");

    double oldTime = -2;
    int index = 0;

    for(auto &y : sim_record_status)
    {
        if(oldTime != y.timeStep)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStep;
            index++;
        }

        fprintf (filePtr, "%-15d", index);
        fprintf (filePtr, "%-15.2f", y.timeStep );

        for(std::string record : record_sim_tokenize)
        {
            if(record == "loaded")
            {
                if(y.loaded != -1)
                    fprintf (filePtr, "%-15ld", y.loaded);
                else
                    fprintf (filePtr, "%-15s", "-");

            }
            else if(record == "departed")
            {
                if(y.departed != -1)
                    fprintf (filePtr, "%-15ld", y.departed);
                else
                    fprintf (filePtr, "%-15s", "-");
            }
            else if(record == "arrived")
            {
                if(y.arrived != -1)
                    fprintf (filePtr, "%-15ld", y.arrived);
                else
                    fprintf (filePtr, "%-15s", "-");
            }
            else if(record == "running")
            {
                if(y.running != -1)
                    fprintf (filePtr, "%-15ld", y.running);
                else
                    fprintf (filePtr, "%-15s", "-");
            }
            else if(record == "waiting")
            {
                if(y.waiting != -1)
                    fprintf (filePtr, "%-15ld", y.waiting);
                else
                    fprintf (filePtr, "%-15s", "-");
            }
            else
            {
                fclose(filePtr);
                throw omnetpp::cRuntimeError("'%s' is not a valid record name", record.c_str());
            }
        }

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}


void Statistics::init_Veh_data(std::string SUMOID, omnetpp::cModule *mod)
{
    auto it = record_status.find(SUMOID);
    if(it == record_status.end())
    {
        bool active = mod->par("record_stat").boolValue();
        std::string record_list = mod->par("record_list").stringValue();

        // make sure record_list is not empty
        if(record_list == "")
            throw omnetpp::cRuntimeError("record_list is empty in vehicle '%s'", SUMOID.c_str());

        // applications are separated by |
        std::vector<std::string> records;
        boost::split(records, record_list, boost::is_any_of("|"));

        // iterate over each application
        std::vector<std::string> record_list_tokenize;
        for(std::string appl : records)
        {
            if(appl == "")
                throw omnetpp::cRuntimeError("Invalid record_list format in vehicle '%s'", SUMOID.c_str());

            // remove leading and trailing spaces from the string
            boost::trim(appl);

            // convert to lower case
            boost::algorithm::to_lower(appl);

            record_list_tokenize.push_back(appl);
        }

        veh_record_list_t entry = {active, record_list_tokenize};
        record_status[SUMOID] = entry;
    }
}


// is called in each time step for each vehicle
void Statistics::record_Veh_data(std::string SUMOID, bool arrived)
{
    auto it = record_status.find(SUMOID);
    if(it == record_status.end())
        return;

    if(!it->second.active)
        return;

    // if vehicle has arrived
    // Note that we cannot call any TraCI commands on this vehicle any more!
    if(arrived)
    {
        // iterate backward over collected_veh_data
        for(auto i = collected_veh_data.rbegin(); i != collected_veh_data.rend(); i++)
        {
            // looking for the last time that we have collected data from this vehicle
            if(i->vehId == SUMOID)
            {
                // then update the arrival/route duration
                i->arrival = omnetpp::simTime().dbl();
                i->routeDuration = i->arrival - i->departure;

                // break out of for loop
                break;
            }
        }

        return;
    }

    veh_data_entry entry = {};

    entry.timeStep = omnetpp::simTime().dbl();

    static int columnNumber = 0;
    for(std::string record : it->second.record_list)
    {
        if(record == "vehid")
            entry.vehId = SUMOID;
        else if(record == "vehtype")
            entry.vehType = TraCI->vehicleGetTypeID(SUMOID);
        else if(record == "lane")
            entry.lane = TraCI->vehicleGetLaneID(SUMOID);
        else if(record == "lanepos")
            entry.lanePos = TraCI->vehicleGetLanePosition(SUMOID);
        else if(record == "pos")
        {
            TraCICoord coord = TraCI->vehicleGetPosition(SUMOID);
            entry.pos = (boost::format("%.2f,%.2f,%.2f") % coord.x % coord.y % coord.z).str();
        }
        else if(record == "speed")
            entry.speed = TraCI->vehicleGetSpeed(SUMOID);
        else if(record == "accel")
            entry.accel = TraCI->vehicleGetCurrentAccel(SUMOID);
        else if(record == "departure")
            entry.departure = TraCI->vehicleGetDepartureTime(SUMOID);
        else if(record == "arrival")
            entry.arrival = TraCI->vehicleGetArrivalTime(SUMOID);
        else if(record == "route")
        {
            // convert vector of string to std::string
            std::string route_edges = "' ";
            for (auto &s : TraCI->vehicleGetRoute(SUMOID)) { route_edges = route_edges + s + " "; }
            route_edges += "'";

            entry.route = route_edges;
        }
        else if(record == "routeduration")
        {
            double departure = TraCI->vehicleGetDepartureTime(SUMOID);
            if(departure != -1)
                entry.routeDuration = (omnetpp::simTime()-departure-updateInterval).dbl();
        }
        else if(record == "drivingdistance")
            entry.drivingDistance = TraCI->vehicleGetDrivingDistance(SUMOID);
        else if(record == "cfmode")
        {
            CFMODES_t CFMode_Enum = TraCI->vehicleGetCarFollowingModelMode(SUMOID);
            switch(CFMode_Enum)
            {
            case Mode_Undefined:
                entry.CFMode = "Undefined";
                break;
            case Mode_NoData:
                entry.CFMode = "NoData";
                break;
            case Mode_DataLoss:
                entry.CFMode = "DataLoss";
                break;
            case Mode_SpeedControl:
                entry.CFMode = "SpeedControl";
                break;
            case Mode_GapControl:
                entry.CFMode = "GapControl";
                break;
            case Mode_EmergencyBrake:
                entry.CFMode = "EmergencyBrake";
                break;
            case Mode_Stopped:
                entry.CFMode = "Stopped";
                break;
            default:
                throw omnetpp::cRuntimeError("Not a valid CFModel!");
                break;
            }
        }
        else if(record == "timegapsetting")
            entry.timeGapSetting = TraCI->vehicleGetTimeGap(SUMOID);
        else if(record == "timegap")
        {
            double speed = (entry.speed != -1) ? entry.speed : TraCI->vehicleGetSpeed(SUMOID);

            auto leader = TraCI->vehicleGetLeader(SUMOID, 900);
            double spaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;

            // calculate timeGap (if leading is present)
            if(leader.leaderID != "" && speed != 0)
                entry.timeGap = spaceGap / speed;
            else
                entry.timeGap = -1;
        }
        else if(record == "frontspacegap")
        {
            auto leader = TraCI->vehicleGetLeader(SUMOID, 900);
            entry.frontSpaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;
        }
        else if(record == "rearspacegap")
        {

        }
        else if(record == "nexttlid")
        {
            std::vector<TL_info_t> res = TraCI->vehicleGetNextTLS(SUMOID);

            if(!res.empty())
                entry.nextTLId = res[0].TLS_id;
            else
                entry.nextTLId = "none";
        }
        else if(record == "nexttllinkstat")
        {
            std::vector<TL_info_t> res = TraCI->vehicleGetNextTLS(SUMOID);

            if(!res.empty())
                entry.nextTLLinkStat = res[0].linkState;
            else
                entry.nextTLLinkStat = 'n';
        }
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'. Check 'record_list' parameter", record.c_str(), SUMOID.c_str());

        auto it = veh_data_columns.find(record);
        if(it == veh_data_columns.end())
        {
            veh_data_columns[record] = columnNumber;
            columnNumber++;
        }
    }

    collected_veh_data.push_back(entry);
}


void Statistics::save_Veh_data_toFile()
{
    if(collected_veh_data.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_vehicleData.txt") % currentRun;

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

    std::string columns_sorted[veh_data_columns.size()];
    for(auto it : veh_data_columns)
        columns_sorted[it.second] = it.first;

    // write column title
    fprintf (filePtr, "%-10s","index");
    fprintf (filePtr, "%-12s","timeStamp");
    for(std::string column : columns_sorted)
        fprintf (filePtr, "%-20s", column.c_str());
    fprintf (filePtr, "\n\n");

    double oldTime = -2;
    int index = 0;

    for(auto &y : collected_veh_data)
    {
        if(oldTime != y.timeStep)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStep;
            index++;
        }

        fprintf (filePtr, "%-10d", index);
        fprintf (filePtr, "%-12.2f", y.timeStep );

        for(std::string record : columns_sorted)
        {
            if(record == "vehid")
            {
                if(y.vehId != "")
                    fprintf (filePtr, "%-20s", y.vehId.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "vehtype")
            {
                if(y.vehType != "")
                    fprintf (filePtr, "%-20s", y.vehType.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "lane")
            {
                if(y.lane != "")
                    fprintf (filePtr, "%-20s", y.lane.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "lanepos")
            {
                if(y.lanePos != -1)
                    fprintf (filePtr, "%-20.2f", y.lanePos);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "pos")
            {
                if(y.pos != "")
                    fprintf (filePtr, "%-20s", y.pos.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "speed")
            {
                if(y.speed != -1)
                    fprintf (filePtr, "%-20.2f", y.speed);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "accel")
            {
                if(y.accel != std::numeric_limits<double>::infinity())
                    fprintf (filePtr, "%-20.2f", y.accel);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "departure")
            {
                if(y.departure != -1)
                    fprintf (filePtr, "%-20.2f", y.departure);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "arrival")
            {
                if(y.arrival != -1)
                    fprintf (filePtr, "%-20.2f", y.arrival);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "route")
            {
                if(y.route != "")
                    fprintf (filePtr, "%-20s", y.route.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "routeduration")
            {
                if(y.routeDuration != -1)
                    fprintf (filePtr, "%-20.2f", y.routeDuration);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "drivingdistance")
            {
                if(y.drivingDistance != -1)
                    fprintf (filePtr, "%-20.2f", y.drivingDistance);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "cfmode")
            {
                if(y.CFMode != "")
                    fprintf (filePtr, "%-20s", y.CFMode.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "timegapsetting")
            {
                if(y.timeGapSetting != -1)
                    fprintf (filePtr, "%-20.2f", y.timeGapSetting);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "timegap")
            {
                if(y.timeGap != std::numeric_limits<double>::infinity())
                    fprintf (filePtr, "%-20.2f", y.timeGap);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "frontspacegap")
            {
                if(y.frontSpaceGap != std::numeric_limits<double>::infinity())
                    fprintf (filePtr, "%-20.2f", y.frontSpaceGap);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "rearspacegap")
            {
                if(y.rearSpaceGap != std::numeric_limits<double>::infinity())
                    fprintf (filePtr, "%-20.2f", y.rearSpaceGap);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "nexttlid")
            {
                if(y.nextTLId != "")
                    fprintf (filePtr, "%-20s", y.nextTLId.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "nexttllinkstat")
            {
                if(y.nextTLLinkStat != '\0')
                    fprintf (filePtr, "%-20c", y.nextTLLinkStat);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else
            {
                fclose(filePtr);
                throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'", record.c_str(), y.vehId.c_str());
            }
        }

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}


void Statistics::init_Ped_data(std::string SUMOID, omnetpp::cModule *mod)
{

}


void Statistics::record_Ped_data(std::string SUMOID, bool arrived)
{

}


void Statistics::init_Veh_emission(std::string SUMOID, omnetpp::cModule *mod)
{
    auto it = record_emission.find(SUMOID);
    if(it == record_emission.end())
    {
        bool active = mod->par("record_emission").boolValue();
        std::string emission_list = mod->par("emission_list").stringValue();

        // make sure emission_list is not empty
        if(emission_list == "")
            throw omnetpp::cRuntimeError("emission_list is empty in vehicle '%s'", SUMOID.c_str());

        // applications are separated by |
        std::vector<std::string> records;
        boost::split(records, emission_list, boost::is_any_of("|"));

        // iterate over each application
        std::vector<std::string> emission_list_tokenize;
        for(std::string appl : records)
        {
            if(appl == "")
                throw omnetpp::cRuntimeError("Invalid emission_list format in vehicle '%s'", SUMOID.c_str());

            // remove leading and trailing spaces from the string
            boost::trim(appl);

            // convert to lower case
            boost::algorithm::to_lower(appl);

            emission_list_tokenize.push_back(appl);
        }

        veh_emission_list_t entry = {active, emission_list_tokenize};
        record_emission[SUMOID] = entry;
    }
}


void Statistics::record_Veh_emission(std::string SUMOID)
{
    auto it = record_emission.find(SUMOID);
    if(it == record_emission.end())
        return;

    if(!it->second.active)
        return;

    veh_emission_entry_t entry = {};

    entry.timeStep = (omnetpp::simTime()-updateInterval).dbl();

    static int columnNumber = 0;
    for(std::string record : it->second.emission_list)
    {
        if(record == "vehid")
            entry.vehId = SUMOID;
        else if(record == "emissionclass")
            entry.emissionClass = TraCI->vehicleGetEmissionClass(SUMOID);
        else if(record == "co2")
            entry.CO2 = TraCI->vehicleGetCO2Emission(SUMOID);
        else if(record == "co")
            entry.CO = TraCI->vehicleGetCOEmission(SUMOID);
        else if(record == "hc")
            entry.HC = TraCI->vehicleGetHCEmission(SUMOID);
        else if(record == "pmx")
            entry.PMx = TraCI->vehicleGetPMxEmission(SUMOID);
        else if(record == "nox")
            entry.NOx = TraCI->vehicleGetNOxEmission(SUMOID);
        else if(record == "fuel")
            entry.fuel = TraCI->vehicleGetFuelConsumption(SUMOID);
        else if(record == "noise")
            entry.noise = TraCI->vehicleGetNoiseEmission(SUMOID);
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'. Check 'emission_list' parameter", record.c_str(), SUMOID.c_str());

        auto it = veh_emission_columns.find(record);
        if(it == veh_emission_columns.end())
        {
            veh_emission_columns[record] = columnNumber;
            columnNumber++;
        }
    }

    collected_veh_emission.push_back(entry);
}


void Statistics::save_Veh_emission_toFile()
{
    if(collected_veh_emission.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_vehicleEmission.txt") % currentRun;

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

    std::string columns_sorted[veh_emission_columns.size()];
    for(auto it : veh_emission_columns)
        columns_sorted[it.second] = it.first;

    // write column title
    fprintf (filePtr, "%-10s","index");
    fprintf (filePtr, "%-12s","timeStamp");
    for(std::string column : columns_sorted)
        fprintf (filePtr, "%-20s", column.c_str());
    fprintf (filePtr, "\n\n");

    double oldTime = -2;
    int index = 0;

    for(auto &y : collected_veh_emission)
    {
        if(oldTime != y.timeStep)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStep;
            index++;
        }

        fprintf (filePtr, "%-10d", index);
        fprintf (filePtr, "%-12.2f", y.timeStep );

        for(std::string record : columns_sorted)
        {
            if(record == "vehid")
            {
                if(y.vehId != "")
                    fprintf (filePtr, "%-20s", y.vehId.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "emissionclass")
            {
                if(y.emissionClass != "")
                    fprintf (filePtr, "%-20s", y.emissionClass.c_str());
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "co2")
            {
                if(y.CO2 != -1)
                    fprintf (filePtr, "%-20.2f", y.CO2);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "co")
            {
                if(y.CO != -1)
                    fprintf (filePtr, "%-20.2f", y.CO);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "hc")
            {
                if(y.HC != -1)
                    fprintf (filePtr, "%-20.2f", y.HC);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "pmx")
            {
                if( y.PMx != -1)
                    fprintf (filePtr, "%-20.2f", y.PMx);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "nox")
            {
                if(y.NOx != -1)
                    fprintf (filePtr, "%-20.2f", y.NOx);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "fuel")
            {
                if(y.fuel != -1)
                    fprintf (filePtr, "%-20.2f", y.fuel);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else if(record == "noise")
            {
                if(y.noise != -1)
                    fprintf (filePtr, "%-20.2f", y.noise);
                else
                    fprintf (filePtr, "%-20s", "-");
            }
            else
            {
                fclose(filePtr);
                throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'", record.c_str(), y.vehId.c_str());
            }
        }

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}

}
