/****************************************************************************/
/// @file    TLStateRecord.cc
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

#include "00_TLStateRecord.h"

namespace VENTOS {

Define_Module(VENTOS::TLStateRecord);

TLStateRecord::~TLStateRecord()
{

}


void TLStateRecord::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();
        maxCycleLength = par("maxCycleLength").doubleValue();

        if(minGreenTime <= 0)
            throw omnetpp::cRuntimeError("minGreenTime value is wrong!");
        if(maxGreenTime <= 0 || maxGreenTime < minGreenTime)
            throw omnetpp::cRuntimeError("maxGreenTime value is wrong!");
        if(yellowTime <= 0)
            throw omnetpp::cRuntimeError("yellowTime value is wrong!");
        if(redTime <= 0)
            throw omnetpp::cRuntimeError("redTime value is wrong!");
        if(maxCycleLength < (minGreenTime + yellowTime + redTime))
            throw omnetpp::cRuntimeError("maxCycleLength value is wrong!");
    }
}


void TLStateRecord::finish()
{
    super::finish();

    saveTLPhasingData();
}


void TLStateRecord::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void TLStateRecord::initialize_withTraCI()
{
    super::initialize_withTraCI();
}


void TLStateRecord::executeEachTimeStep()
{
    super::executeEachTimeStep();
}


void TLStateRecord::updateTLstate(std::string TLid, std::string stage, std::string currentInterval, bool cycleStart)
{
    if(stage == "init")
    {
        // initialize phase number in this TL
        phaseTL[TLid] = 1;

        // initialize status in this TL
        currentStatusTL_t entry = {};

        entry.cycle = 1;
        entry.allowedMovements = currentInterval;
        entry.greenLength = -1;
        entry.greenStart = omnetpp::simTime().dbl();
        entry.yellowStart = -1;
        entry.redStart = -1;
        entry.phaseEnd = -1;

        statusTL.insert( std::make_pair(std::make_pair(TLid,1), entry) );
    }
    else
    {
        auto it = phaseTL.find(TLid);
        if(it == phaseTL.end())
            throw omnetpp::cRuntimeError("Cannot find the current phase in TL '%s'", TLid.c_str());

        // get the current phase
        int currentPhase = it->second;

        auto it2 = statusTL.find(std::make_pair(TLid,currentPhase));
        if(it2 == statusTL.end())
            throw omnetpp::cRuntimeError("Cannot find status for TL '%s' at phase '%d'", TLid.c_str(), currentPhase);

        currentStatusTL_t &status = it2->second;

        if(stage == "yellow")
        {
            status.yellowStart = omnetpp::simTime().dbl();
            status.greenLength = status.yellowStart - status.greenStart;

            // get green duration
            double green_duration = status.yellowStart - status.greenStart;

            // todo: make sure green interval is above G_min
            // if(green_duration - minGreenTime < 0)
            //     throw omnetpp::cRuntimeError("green interval is less than minGreenTime = %0.3f", minGreenTime);
            //
            // make sure green interval is below G_max
            // if(green_duration - maxGreenTime > 0)
            //     throw omnetpp::cRuntimeError("green interval is greater than maxGreenTime = %0.3f", maxGreenTime);
        }
        else if(stage == "red")
        {
            status.redStart = omnetpp::simTime().dbl();

            // get yellow duration
            double yellow_duration = status.redStart - status.yellowStart;

            // todo:
            // if( fabs(yellow_duration - yellowTime) < 0.0001 )
            //     throw omnetpp::cRuntimeError("yellow interval is not %0.3f", yellowTime);
        }
        else if(stage == "phaseEnd")
        {
            // update TL status for this phase
            status.phaseEnd = omnetpp::simTime().dbl();

            // get red duration
            double red_duration = status.phaseEnd - status.redStart;

            // todo:
            // if(red_duration - redTime != 0)
            //     throw omnetpp::cRuntimeError("red interval is not %0.3f", redTime);

            // get the current cycle number
            int cycleNumber = status.cycle;

            // on a new cycle
            if(cycleStart)
            {
                // todo: check if cycle length is ok?

                // increase cycle number by 1
                cycleNumber++;

                // update traffic demand at the beginning of each cycle
                updateTrafficDemand();
            }

            // increase phase number by 1
            auto location2 = phaseTL.find(TLid);
            location2->second = location2->second + 1;

            // update status for the new phase
            currentStatusTL_t entry = {};

            entry.cycle = cycleNumber;
            entry.allowedMovements = currentInterval;
            entry.greenLength = -1;
            entry.greenStart = omnetpp::simTime().dbl();
            entry.yellowStart = -1;
            entry.redStart = -1;
            entry.phaseEnd = -1;

            statusTL.insert( std::make_pair(std::make_pair(TLid,location2->second), entry) );
        }
        else throw omnetpp::cRuntimeError("stage is not recognized!");
    }
}


const TLStateRecord::currentStatusTL_t TLStateRecord::TLGetStatus(std::string TLid)
{
    auto it = phaseTL.find(TLid);
    if(it == phaseTL.end())
        throw omnetpp::cRuntimeError("Cannot find the current phase in TL '%s'", TLid.c_str());

    // get the current phase
    int currentPhase = it->second;

    auto it2 = statusTL.find(std::make_pair(TLid,currentPhase));
    if(it2 == statusTL.end())
        throw omnetpp::cRuntimeError("Cannot find status for TL '%s' at phase '%d'", TLid.c_str(), currentPhase);

    return it2->second;
}


void TLStateRecord::saveTLPhasingData()
{
    if(statusTL.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_TLphasingData.txt") % currentRun;

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
    fprintf (filePtr, "%-12s", "TLid");
    fprintf (filePtr, "%-12s", "phase");
    fprintf (filePtr, "%-12s", "cycle");
    fprintf (filePtr, "%-35s", "allowedMovements");
    fprintf (filePtr, "%-15s", "greenLength");
    fprintf (filePtr, "%-15s", "greenStart");
    fprintf (filePtr, "%-15s", "yellowStart");
    fprintf (filePtr, "%-15s", "redStart");
    fprintf (filePtr, "%-15s \n\n", "phaseEnd");

    // write body
    for(auto &y : statusTL)
    {
        currentStatusTL status = y.second;

        fprintf (filePtr, "%-12s", y.first.first.c_str());
        fprintf (filePtr, "%-12d", y.first.second);
        fprintf (filePtr, "%-12d", status.cycle);
        fprintf (filePtr, "%-35s", status.allowedMovements.c_str());
        fprintf (filePtr, "%-15.2f", status.greenLength);
        fprintf (filePtr, "%-15.2f", status.greenStart);
        fprintf (filePtr, "%-15.2f", status.yellowStart);
        fprintf (filePtr, "%-15.2f", status.redStart);
        fprintf (filePtr, "%-15.2f \n", status.phaseEnd);
    }

    fclose(filePtr);
}

}
