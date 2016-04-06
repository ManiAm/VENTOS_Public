/****************************************************************************/
/// @file    Mac1609_4_Mod.cc
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

#include "Mac1609_4_Mod.h"
#include "SignalObj.h"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

std::vector<MacStatEntry> Mac1609_4_Mod::Vec_MacStat;

Define_Module(VENTOS::Mac1609_4_Mod);

void Mac1609_4_Mod::initialize(int stage)
{
    Mac1609_4::initialize(stage);

    if (stage == 0)
    {
        reportMAClayerData = par("reportMAClayerData").boolValue();
        Vec_MacStat.clear();
    }
}


void Mac1609_4_Mod::finish()
{
    Mac1609_4::finish();

    // run this code only once
    static bool wasExecuted = false;
    if(reportMAClayerData && !wasExecuted)
    {
        MAClayerToFile();
        wasExecuted = true;
    }
}


void Mac1609_4_Mod::handleSelfMsg(cMessage* msg)
{
    Mac1609_4::handleSelfMsg(msg);
}


void Mac1609_4_Mod::handleUpperControl(cMessage* msg)
{
    Mac1609_4::handleUpperControl(msg);
}


// all messages from application layer are sent to this method!
void Mac1609_4_Mod::handleUpperMsg(cMessage* msg)
{
    Mac1609_4::handleUpperMsg(msg);

    if(!reportMAClayerData)
        return;

    std::vector<long> MacStats;

    MacStats.push_back(statsDroppedPackets);        // packet was dropped in Mac
    MacStats.push_back(statsNumTooLittleTime);      // Too little time in this interval. Will not schedule nextMacEvent
    MacStats.push_back(statsNumInternalContention); // there was already another packet ready.
    // we have to go increase CW and go into backoff.
    // It's called internal contention and its wonderful
    MacStats.push_back(statsNumBackoff);
    MacStats.push_back(statsSlotsBackoff);
    MacStats.push_back(statsTotalBusyTime.dbl());

    MacStats.push_back(statsSentPackets);

    MacStats.push_back(statsSNIRLostPackets);     // A packet was not received due to biterrors
    MacStats.push_back(statsTXRXLostPackets);     // A packet was not received because we were sending while receiving

    MacStats.push_back(statsReceivedPackets);     // Received a data packet addressed to me
    MacStats.push_back(statsReceivedBroadcasts);  // Received a broadcast data packet

    std::string name = this->getParentModule()->getParentModule()->getFullName();
    const MacStatEntry *searchFor = new MacStatEntry(-1, name, std::vector<long>());
    auto counter = std::find(Vec_MacStat.begin(), Vec_MacStat.end(), *searchFor);

    // its a new entry, so we add it
    if(counter == Vec_MacStat.end())
    {
        MacStatEntry *tmp = new MacStatEntry(simTime().dbl(), name, MacStats);
        Vec_MacStat.push_back(*tmp);
    }
    // if found, just update the existing fields
    else
    {
        counter->time = simTime().dbl();
        counter->MacStatsVec = MacStats;
    }
}


void Mac1609_4_Mod::handleLowerControl(cMessage* msg)
{
    Mac1609_4::handleLowerControl(msg);
}


void Mac1609_4_Mod::handleLowerMsg(cMessage* msg)
{
    Mac1609_4::handleLowerMsg(msg);
}


void Mac1609_4_Mod::MAClayerToFile()
{
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

}

