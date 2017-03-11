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
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // register signals
        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        record_sim_stat = par("record_sim_stat").boolValue();
    }
}


void Statistics::finish()
{
    save_beacon_stat_toFile();

    save_plnManage_toFile();
    save_plnStat_toFile();

    save_MAC_stat_toFile();
    save_PHY_stat_toFile();
    save_FrameTxRx_stat_toFile();

    save_Sim_data_toFile();
    save_Veh_data_toFile();

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
}


void Statistics::handleMessage(omnetpp::cMessage *msg)
{

}


void Statistics::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        Statistics::initialize_withTraCI();
    }
}


void Statistics::initialize_withTraCI()
{
    updateInterval = (double)TraCI->simulationGetTimeStep() / 1000.;
}


void Statistics::executeEachTimestep()
{

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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
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


void Statistics::save_plnManage_toFile()
{
    if(global_plnManagement_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_plnManage.txt") % currentRun;

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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-15s","sender");
    fprintf (filePtr, "%-17s","receiver");
    fprintf (filePtr, "%-25s","type");
    fprintf (filePtr, "%-20s","sendingPlnID");
    fprintf (filePtr, "%-20s\n\n","recPlnID");

    std::string oldSender = "";
    double oldTime = -1;

    // write body
    for(auto &y : global_plnManagement_stat)
    {
        // make the log more readable :)
        if(y.sender != oldSender || y.time != oldTime)
        {
            fprintf(filePtr, "\n");
            oldSender = y.sender;
            oldTime = y.time;
        }

        fprintf (filePtr, "%-10.2f", y.time);
        fprintf (filePtr, "%-15s", y.sender.c_str());
        fprintf (filePtr, "%-17s", y.receiver.c_str());
        fprintf (filePtr, "%-30s", y.type.c_str());
        fprintf (filePtr, "%-18s", y.sendingPlnID.c_str());
        fprintf (filePtr, "%-20s\n", y.receivingPlnID.c_str());
    }

    fclose(filePtr);
}


void Statistics::save_plnStat_toFile()
{
    if(global_plnData_stat.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_plnStat.txt") % currentRun;

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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from platoon");
    fprintf (filePtr, "%-20s","to platoon");
    fprintf (filePtr, "%-20s\n\n","comment");

    std::string oldPln = "";

    // write body
    for(auto &y : global_plnData_stat)
    {
        if(y.from != oldPln)
        {
            fprintf(filePtr, "\n");
            oldPln = y.from;
        }

        fprintf (filePtr, "%-10.2f", y.time);
        fprintf (filePtr, "%-20s", y.from.c_str());
        fprintf (filePtr, "%-20s", y.to.c_str());
        fprintf (filePtr, "%-20s\n", y.maneuver.c_str());
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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","lastStatTime");
    fprintf (filePtr, "%-20s","DroppedPackets");
    fprintf (filePtr, "%-20s","NumTooLittleTime");
    fprintf (filePtr, "%-30s","NumInternalContention");
    fprintf (filePtr, "%-20s","NumBackoff");
    fprintf (filePtr, "%-20s","SlotsBackoff");
    fprintf (filePtr, "%-20s","TotalBusyTime");
    fprintf (filePtr, "%-20s","SentPackets");
    fprintf (filePtr, "%-20s","ReceivedPackets");
    fprintf (filePtr, "%-20s\n\n","ReceivedBroadcasts");

    // write body
    for(auto &y : global_MAC_stat)
    {
        fprintf (filePtr, "%-20s", y.first.c_str());
        fprintf (filePtr, "%-20.8f", y.second.last_stat_time);
        fprintf (filePtr, "%-20ld", y.second.statsDroppedPackets);
        fprintf (filePtr, "%-20ld", y.second.statsNumTooLittleTime);
        fprintf (filePtr, "%-30ld", y.second.statsNumInternalContention);
        fprintf (filePtr, "%-20ld", y.second.statsNumBackoff);
        fprintf (filePtr, "%-20ld", y.second.statsSlotsBackoff);
        fprintf (filePtr, "%-20.8f", y.second.statsTotalBusyTime);
        fprintf (filePtr, "%-20ld", y.second.statsSentPackets);
        fprintf (filePtr, "%-20ld", y.second.statsReceivedPackets);
        fprintf (filePtr, "%-20ld\n", y.second.statsReceivedBroadcasts);
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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","lastStatTime");
    fprintf (filePtr, "%-15s","SentFrames");
    fprintf (filePtr, "%-20s","ReceivedFrames");
    fprintf (filePtr, "%-20s","BiteErrorLostFrames");
    fprintf (filePtr, "%-20s","CollisionLostFrames");
    fprintf (filePtr, "%-20s\n\n","TXRXLostFrames");

    // write body
    for(auto &y : global_PHY_stat)
    {
        fprintf (filePtr, "%-20s", y.first.c_str());
        fprintf (filePtr, "%-20.8f", y.second.last_stat_time);
        fprintf (filePtr, "%-15ld", y.second.statsSentFrames);
        fprintf (filePtr, "%-20ld", y.second.statsReceivedFrames);
        fprintf (filePtr, "%-20ld", y.second.statsBiteErrorLostFrames);
        fprintf (filePtr, "%-20ld", y.second.statsCollisionLostFrames);
        fprintf (filePtr, "%-20ld\n", y.second.statsTXRXLostFrames);
    }

    fclose(filePtr);
}


void Statistics::save_FrameTxRx_stat_toFile()
{
    if(global_frameTxRx_stat.empty())
        return;

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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
    }

    // write header
    fprintf (filePtr, "%-20s","msgId");
    fprintf (filePtr, "%-20s","msgName");
    fprintf (filePtr, "%-20s","senderNode");
    fprintf (filePtr, "%-20s","receiverNode");
    fprintf (filePtr, "%-20s","receiverGateId");
    fprintf (filePtr, "%-20s","frameSize(bytes)");
    fprintf (filePtr, "%-20s","sentAt(s)");
    fprintf (filePtr, "%-20s","txSpeed(Mbps)");
    fprintf (filePtr, "%-20s","txTime(ms)");
    fprintf (filePtr, "%-22s","propagationDelay(ms)");
    fprintf (filePtr, "%-20s","receivedAt(s)");
    fprintf (filePtr, "%-20s\n\n","rxStatus");

    // write body
    for(auto &y : global_frameTxRx_stat)
    {
        long int msgId = y.first.first;
        long int nic = y.first.second;

        fprintf (filePtr, "%-20ld", msgId);
        fprintf (filePtr, "%-20s", y.second.msgName.c_str());
        fprintf (filePtr, "%-20s", y.second.senderNode.c_str());
        fprintf (filePtr, "%-20s", y.second.receiverNode.c_str());
        fprintf (filePtr, "%-20ld", nic);
        fprintf (filePtr, "%-20d", y.second.frameSize);
        fprintf (filePtr, "%-20.8f", y.second.sentAt);
        fprintf (filePtr, "%-20.2f", y.second.TxSpeed);
        fprintf (filePtr, "%-20.8f", y.second.TxTime);
        fprintf (filePtr, "%-22.13f", y.second.propagationDelay);
        fprintf (filePtr, "%-20.8f", y.second.receivedAt);
        fprintf (filePtr, "%-20s\n", y.second.receivedStatus.c_str());
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
    entry.loaded = -1;
    entry.departed = -1;
    entry.arrived = -1;
    entry.running = -1;
    entry.waiting = -1;

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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
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
                fprintf (filePtr, "%-15ld", y.loaded);
            else if(record == "departed")
                fprintf (filePtr, "%-15ld", y.departed);
            else if(record == "arrived")
                fprintf (filePtr, "%-15ld", y.arrived);
            else if(record == "running")
                fprintf (filePtr, "%-15ld", y.running);
            else if(record == "waiting")
                fprintf (filePtr, "%-15ld", y.waiting);
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
void Statistics::record_Veh_data(std::string vID, bool arrived)
{
    auto it = record_status.find(vID);
    if(it == record_status.end())
        return;

    if(!it->second.active)
        return;

    for(auto i = collected_veh_data.rbegin(); i != collected_veh_data.rend(); i++)
    {
        // looking for the last entry of vID
        if(i->vehId == vID)
        {
            // if vehicle has arrived
            if(arrived)
            {
                // then update the arrival/route duration
                i->arrival = (omnetpp::simTime()-updateInterval).dbl();
                i->routeDuration = i->arrival - i->departure;
                return;
            }
            else
            {
                // make sure that the arrival time is -1
                if(i->arrival == -1)
                    break;

                throw omnetpp::cRuntimeError("veh '%s' has already arrived!", vID.c_str());
            }
        }
    }

    veh_data_entry entry = {};

    entry.timeStep = (omnetpp::simTime()-updateInterval).dbl();
    entry.vehId = "n/a";
    entry.vehType = "n/a";
    entry.lane = "n/a";
    entry.lanePos = -1;
    entry.speed = -1;
    entry.accel = std::numeric_limits<double>::infinity();
    entry.departure = 0;
    entry.arrival = -1;
    entry.route = "";
    entry.routeDuration = 0;
    entry.drivingDistance = -1;
    entry.CFMode = "n/a";
    entry.timeGapSetting = -1;
    entry.timeGap = -2;
    entry.frontSpaceGap = -2;
    entry.rearSpaceGap = -2;
    entry.nextTLId = "n/a";
    entry.nextTLLinkStat = '\0';

    static int columnNumber = 0;
    for(std::string record : it->second.record_list)
    {
        if(record == "vehid")
            entry.vehId = vID;
        else if(record == "vehtype")
            entry.vehType = TraCI->vehicleGetTypeID(vID);
        else if(record == "lane")
            entry.lane = TraCI->vehicleGetLaneID(vID);
        else if(record == "lanepos")
            entry.lanePos = TraCI->vehicleGetLanePosition(vID);
        else if(record == "speed")
            entry.speed = TraCI->vehicleGetSpeed(vID);
        else if(record == "accel")
            entry.accel = TraCI->vehicleGetCurrentAccel(vID);
        else if(record == "departure")
            entry.departure = TraCI->vehicleGetDepartureTime(vID);
        else if(record == "arrival")
            entry.arrival = TraCI->vehicleGetArrivalTime(vID);
        else if(record == "route")
        {
            // convert vector of string to std::string
            std::string route_edges = "' ";
            for (auto &s : TraCI->vehicleGetRoute(vID)) { route_edges = route_edges + s + " "; }
            route_edges += "'";

            entry.route = route_edges;
        }
        else if(record == "routeduration")
        {
            double departure = TraCI->vehicleGetDepartureTime(vID);
            if(departure != -1)
                entry.routeDuration = (omnetpp::simTime()-departure-updateInterval).dbl();
        }
        else if(record == "drivingdistance")
            entry.drivingDistance = TraCI->vehicleGetDrivingDistance(vID);
        else if(record == "cfmode")
        {
            CFMODES_t CFMode_Enum = TraCI->vehicleGetCarFollowingMode(vID);
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
            entry.timeGapSetting = TraCI->vehicleGetTimeGap(vID);
        else if(record == "timegap")
        {
            double speed = (entry.speed != -1) ? entry.speed : TraCI->vehicleGetSpeed(vID);

            auto leader = TraCI->vehicleGetLeader(vID, 900);
            double spaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;

            // calculate timeGap (if leading is present)
            if(leader.leaderID != "" && speed != 0)
                entry.timeGap = spaceGap / speed;
            else
                entry.timeGap = -1;
        }
        else if(record == "frontspacegap")
        {
            auto leader = TraCI->vehicleGetLeader(vID, 900);
            entry.frontSpaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;
        }
        else if(record == "rearspacegap")
        {

        }
        else if(record == "nexttlid")
        {
            std::vector<TL_info_t> res = TraCI->vehicleGetNextTLS(vID);

            if(!res.empty())
                entry.nextTLId = res[0].TLS_id;
            else
                entry.nextTLId = "";
        }
        else if(record == "nexttllinkstat")
        {
            std::vector<TL_info_t> res = TraCI->vehicleGetNextTLS(vID);

            if(!res.empty())
                entry.nextTLLinkStat = res[0].linkState;
            else
                entry.nextTLLinkStat = 'n';
        }
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'", record.c_str(), vID.c_str());

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

    // file name for saving vehicles statistics
    std::string veh_stat_file = par("veh_stat_file").stringValue();

    boost::filesystem::path filePath ("results");

    // no file name specified
    if(veh_stat_file == "")
    {
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        std::ostringstream fileName;
        fileName << boost::format("%03d_vehicleData.txt") % currentRun;

        filePath /= fileName.str();
    }
    else
        filePath /= veh_stat_file;

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
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration().c_str());
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
                fprintf (filePtr, "%-20s", y.vehId.c_str());
            else if(record == "vehtype")
                fprintf (filePtr, "%-20s", y.vehType.c_str());
            else if(record == "lane")
                fprintf (filePtr, "%-20s", y.lane.c_str());
            else if(record == "lanepos")
                fprintf (filePtr, "%-20.2f", y.lanePos);
            else if(record == "speed")
                fprintf (filePtr, "%-20.2f", y.speed);
            else if(record == "accel")
                fprintf (filePtr, "%-20.2f", y.accel);
            else if(record == "departure")
                fprintf (filePtr, "%-20.2f", y.departure);
            else if(record == "arrival")
                fprintf (filePtr, "%-20.2f", y.arrival);
            else if(record == "route")
                fprintf (filePtr, "%-20s", y.route.c_str());
            else if(record == "routeduration")
                fprintf (filePtr, "%-20.2f", y.routeDuration);
            else if(record == "drivingdistance")
                fprintf (filePtr, "%-20.2f", y.drivingDistance);
            else if(record == "cfmode")
                fprintf (filePtr, "%-20s", y.CFMode.c_str());
            else if(record == "timegapsetting")
                fprintf (filePtr, "%-20.2f", y.timeGapSetting);
            else if(record == "timegap")
                fprintf (filePtr, "%-20.2f", y.timeGap);
            else if(record == "frontspacegap")
                fprintf (filePtr, "%-20.2f", y.frontSpaceGap);
            else if(record == "rearspacegap")
                fprintf (filePtr, "%-20.2f", y.rearSpaceGap);
            else if(record == "nexttlid")
                fprintf (filePtr, "%-20s", y.nextTLId.c_str());
            else if(record == "nexttllinkstat")
                fprintf (filePtr, "%-20c", y.nextTLLinkStat);
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


void Statistics::init_Veh_emission(std::string SUMOID, omnetpp::cModule *mod)
{

}


void Statistics::record_Veh_emission(std::string vID, bool arrived)
{

}


void Statistics::save_Veh_emission_toFile()
{

}

}
