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

#include <Statistics.h>
#include <ApplRSU_05_Manager.h>
#include "SignalObj.h"

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
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        reportMAClayerData = par("reportMAClayerData").boolValue();
        reportPlnManagerData = par("reportPlnManagerData").boolValue();
        reportBeaconsData = par("reportBeaconsData").boolValue();

        // register signals
        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        Signal_MacStats = registerSignal("MacStats");
        simulation.getSystemModule()->subscribe("MacStats", this);

        Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
        simulation.getSystemModule()->subscribe("SentPlatoonMsg", this);

        Signal_VehicleState = registerSignal("VehicleState");
        simulation.getSystemModule()->subscribe("VehicleState", this);

        Signal_PlnManeuver = registerSignal("PlnManeuver");
        simulation.getSystemModule()->subscribe("PlnManeuver", this);

        Signal_beacon = registerSignal("beacon");
        simulation.getSystemModule()->subscribe("beacon", this);
    }
}


void Statistics::finish()
{
    if(reportMAClayerData)
        MAClayerToFile();

    if(reportBeaconsData)
        beaconToFile();

    if(reportPlnManagerData)
    {
        plnManageToFile();
        plnStatToFile();
    }
}


void Statistics::handleMessage(cMessage *msg)
{

}


void Statistics::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        Statistics::executeEachTimestep();
    }
    else if(signalID == Signal_executeFirstTS)
    {
        Statistics::executeFirstTimeStep();
    }
}


void Statistics::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();

    if(reportMAClayerData && signalID == Signal_MacStats)
    {
        MacStat *m = static_cast<MacStat *>(obj);
        if (m == NULL) return;

        const MacStatEntry *searchFor = new MacStatEntry(-1, source->getFullName(), std::vector<long>());
        auto counter = std::find(Vec_MacStat.begin(), Vec_MacStat.end(), *searchFor);

        // its a new entry, so we add it
        if(counter == Vec_MacStat.end())
        {
            MacStatEntry *tmp = new MacStatEntry(simTime().dbl(), source->getFullName(), m->vec);
            Vec_MacStat.push_back(*tmp);
        }
        // if found, just update the existing fields
        else
        {
            counter->time = simTime().dbl();
            counter->MacStatsVec = m->vec;
        }
    }
    else if(reportPlnManagerData && signalID == Signal_VehicleState)
    {
        CurrentVehicleState *state = dynamic_cast<CurrentVehicleState*>(obj);
        ASSERT(state);

        plnManagement *tmp = new plnManagement(simTime().dbl(), state->name, "-", state->state, "-", "-");
        Vec_plnManagement.push_back(*tmp);
    }
    else if(reportPlnManagerData && signalID == Signal_SentPlatoonMsg)
    {
        CurrentPlnMsg* plnMsg = dynamic_cast<CurrentPlnMsg*>(obj);
        ASSERT(plnMsg);

        plnManagement *tmp = new plnManagement(simTime().dbl(), plnMsg->msg->getSender(), plnMsg->msg->getRecipient(), plnMsg->type, plnMsg->msg->getSendingPlatoonID(), plnMsg->msg->getReceivingPlatoonID());
        Vec_plnManagement.push_back(*tmp);
    }
    else if(reportPlnManagerData && signalID == Signal_PlnManeuver)
    {
        PlnManeuver* com = dynamic_cast<PlnManeuver*>(obj);
        ASSERT(com);

        plnStat *tmp = new plnStat(simTime().dbl(), com->from, com->to, com->maneuver);
        Vec_plnStat.push_back(*tmp);
    }
    else if(reportBeaconsData && signalID == Signal_beacon)
    {
        data *m = static_cast<data *>(obj);
        ASSERT(m);

        BeaconStat *tmp = new BeaconStat(simTime().dbl(), m->sender, m->receiver, m->dropped);
        Vec_Beacons.push_back(*tmp);
    }
}


void Statistics::executeFirstTimeStep()
{

}


void Statistics::executeEachTimestep()
{

}


void Statistics::MAClayerToFile()
{
    if(Vec_MacStat.empty())
        return;

    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/MACdata.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_MACdata.txt";
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
    fprintf (filePtr, "%-20s","timeStep");
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","DroppedPackets");
    fprintf (filePtr, "%-20s","NumTooLittleTime");
    fprintf (filePtr, "%-30s","NumInternalContention");
    fprintf (filePtr, "%-20s","NumBackoff");
    fprintf (filePtr, "%-20s","SlotsBackoff");
    fprintf (filePtr, "%-20s","TotalBusyTime");
    fprintf (filePtr, "%-20s","SentPackets");
    fprintf (filePtr, "%-20s","SNIRLostPackets");
    fprintf (filePtr, "%-20s","TXRXLostPackets");
    fprintf (filePtr, "%-20s","ReceivedPackets");
    fprintf (filePtr, "%-20s\n\n","ReceivedBroadcasts");

    // write body
    for(auto &y : Vec_MacStat)
    {
        fprintf (filePtr, "%-20.2f ", y.time);
        fprintf (filePtr, "%-20s ", y.name.c_str());
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[0]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[1]);
        fprintf (filePtr, "%-30ld ", y.MacStatsVec[2]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[3]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[4]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[5]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[6]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[7]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[8]);
        fprintf (filePtr, "%-20ld ", y.MacStatsVec[9]);
        fprintf (filePtr, "%-20ld\n", y.MacStatsVec[10]);
    }

    fclose(filePtr);
}


void Statistics::plnManageToFile()
{
    if(Vec_plnManagement.empty())
        return;

    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/plnManage.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_plnManage.txt";
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
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-15s","sender");
    fprintf (filePtr, "%-17s","receiver");
    fprintf (filePtr, "%-25s","type");
    fprintf (filePtr, "%-20s","sendingPlnID");
    fprintf (filePtr, "%-20s\n\n","recPlnID");

    std::string oldSender = "";
    double oldTime = -1;

    // write body
    for(auto &y : Vec_plnManagement)
    {
        // make the log more readable :)
        if(y.sender != oldSender || y.time != oldTime)
        {
            fprintf(filePtr, "\n");
            oldSender = y.sender;
            oldTime = y.time;
        }

        fprintf (filePtr, "%-10.2f ", y.time);
        fprintf (filePtr, "%-15s ", y.sender.c_str());
        fprintf (filePtr, "%-17s ", y.receiver.c_str());
        fprintf (filePtr, "%-30s ", y.type.c_str());
        fprintf (filePtr, "%-18s ", y.sendingPlnID.c_str());
        fprintf (filePtr, "%-20s\n", y.receivingPlnID.c_str());
    }

    fclose(filePtr);
}


void Statistics::plnStatToFile()
{
    if(Vec_plnStat.empty())
        return;

    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/plnStat.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_plnStat.txt";
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
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from platoon");
    fprintf (filePtr, "%-20s","to platoon");
    fprintf (filePtr, "%-20s\n\n","comment");

    std::string oldPln = "";

    // write body
    for(auto &y : Vec_plnStat)
    {
        if(y.from != oldPln)
        {
            fprintf(filePtr, "\n");
            oldPln = y.from;
        }

        fprintf (filePtr, "%-10.2f ", y.time);
        fprintf (filePtr, "%-20s ", y.from.c_str());
        fprintf (filePtr, "%-20s ", y.to.c_str());
        fprintf (filePtr, "%-20s\n", y.maneuver.c_str());
    }

    fclose(filePtr);
}


void Statistics::beaconToFile()
{
    if(Vec_Beacons.empty())
        return;

    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/beaconsStat.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_beaconsStat.txt";
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
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from");
    fprintf (filePtr, "%-20s","to");
    fprintf (filePtr, "%-20s\n\n","dropped");

    for(auto &y : Vec_Beacons)
    {
        fprintf (filePtr, "%-12.2f ", y.time);
        fprintf (filePtr, "%-20s ", y.senderID.c_str());
        fprintf (filePtr, "%-20s ", y.receiverID.c_str());
        fprintf (filePtr, "%-20d \n", y.dropped);
    }

    fclose(filePtr);
}


// returns the index of a node. For example gets V[10] as input and returns 10
int Statistics::getNodeIndex(std::string ModName)
{
    std::ostringstream oss;

    for(unsigned int h=0; h < ModName.length(); h++)
    {
        if ( isdigit(ModName[h]) )
            oss << ModName[h];
    }

    // return the node ID
    return atoi(oss.str().c_str());
}


}
