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

#include <Statistics.h>
#include <ApplRSU_06_Manager.h>
#include "Router.h"
#include "SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::Statistics);

class ApplRSUManager;


Statistics::~Statistics()
{

}


void Statistics::initialize(int stage)
{
    if(stage == 0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);
        ASSERT(TraCI);

        terminate = module->par("terminate").doubleValue();
        updateInterval = module->par("updateInterval").doubleValue();

        cModule *rmodule = simulation.getSystemModule()->getSubmodule("router");
        router = static_cast< Router* >(rmodule);

        collectVehiclesData = par("collectVehiclesData").boolValue();
        vehicleDataLevel = par("vehicleDataLevel").longValue();

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
    if(collectVehiclesData)
        vehiclesDataToFile();

    if(reportMAClayerData)
        MAClayerToFile();

    if(reportPlnManagerData)
    {
        plnManageToFile();
        plnStatToFile();
    }

    if(reportBeaconsData)
        beaconToFile();
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

    int nodeIndex = getNodeIndex(source->getFullName());

    if(reportMAClayerData && signalID == Signal_MacStats)
    {
        MacStat *m = static_cast<MacStat *>(obj);
        if (m == NULL) return;

        const MacStatEntry *searchFor = new MacStatEntry(-1, source->getFullName(), -1, std::vector<long>());
        auto counter = std::find(Vec_MacStat.begin(), Vec_MacStat.end(), *searchFor);

        // its a new entry, so we add it
        if(counter == Vec_MacStat.end())
        {
            MacStatEntry *tmp = new MacStatEntry(simTime().dbl(), source->getFullName(), nodeIndex, m->vec);
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
    // get the list of all TL
    TLList = TraCI->TLGetIDList();

    // get all lanes in the network
    lanesList = TraCI->laneGetIDList();
}


void Statistics::executeEachTimestep()
{
    if(collectVehiclesData)
        vehiclesData();   // collecting data from all vehicles in each timeStep
}


void Statistics::vehiclesData()
{
    for(auto &i : lanesList)
    {
        // get all vehicles on lane i
        std::list<std::string> allVeh = TraCI->laneGetLastStepVehicleIDs( i.c_str() );

        for(std::list<std::string>::reverse_iterator k = allVeh.rbegin(); k != allVeh.rend(); ++k)
            saveVehicleData(k->c_str());
    }
}


void Statistics::saveVehicleData(std::string vID)
{
    double timeStep = -1;
    std::string vType = "n/a";
    std::string lane = "n/a";
    double pos = -1;
    double speed = -1;
    double accel = std::numeric_limits<double>::infinity();
    std::string CFMode = "n/a";
    double timeGapSetting = -1;
    double spaceGap = -2;
    double timeGap = -2;
    std::string TLid = "n/a";   // TLid that controls this vehicle. Empty string means the vehicle is not controlled by any TLid
    char linkStatus = '\0';     // status of the TL ahead (character 'n' means no TL ahead)

    // full collection
    if(vehicleDataLevel == 1)
    {
        timeStep = (simTime()-updateInterval).dbl();
        vType = TraCI->vehicleGetTypeID(vID);
        lane = TraCI->vehicleGetLaneID(vID);
        pos = TraCI->vehicleGetLanePosition(vID);
        speed = TraCI->vehicleGetSpeed(vID);
        accel = TraCI->vehicleGetCurrentAccel(vID);

        int CFMode_Enum = TraCI->vehicleGetCarFollowingMode(vID);
        switch(CFMode_Enum)
        {
        case Mode_Undefined:
            CFMode = "Undefined";
            break;
        case Mode_NoData:
            CFMode = "NoData";
            break;
        case Mode_DataLoss:
            CFMode = "DataLoss";
            break;
        case Mode_SpeedControl:
            CFMode = "SpeedControl";
            break;
        case Mode_GapControl:
            CFMode = "GapControl";
            break;
        case Mode_EmergencyBrake:
            CFMode = "EmergencyBrake";
            break;
        case Mode_Stopped:
            CFMode = "Stopped";
            break;
        default:
            error("Not a valid CFModel!");
            break;
        }

        // get the timeGap setting
        timeGapSetting = TraCI->vehicleGetTimeGap(vID);

        // get the space gap
        std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(vID, 900);
        std::string vleaderID = vleaderIDnew[0];
        spaceGap = (vleaderID != "") ? atof(vleaderIDnew[1].c_str()) : -1;

        // calculate timeGap (if leading is present)
        if(vleaderID != "" && speed != 0)
            timeGap = spaceGap / speed;
        else
            timeGap = -1;

        // get the TLid that controls this vehicle
        // empty string means the vehicle is not controlled by any TLid
        TLid =TraCI->vehicleGetTLID(vID);

        // get the signal status ahead
        // character 'n' means no link status
        linkStatus = TraCI->vehicleGetTLLinkStatus(vID);
    }
    // router scenario
    else if(vehicleDataLevel == 2)
    {
        timeStep = (simTime()-updateInterval).dbl();
        vType = TraCI->vehicleGetTypeID(vID);
        lane = TraCI->vehicleGetLaneID(vID);
        pos = TraCI->vehicleGetLanePosition(vID);
        speed = TraCI->vehicleGetSpeed(vID);
        accel = TraCI->vehicleGetCurrentAccel(vID);
    }
    // traffic light scenario
    else if(vehicleDataLevel == 3)
    {
        timeStep = (simTime()-updateInterval).dbl();
        lane = TraCI->vehicleGetLaneID(vID);
        speed = TraCI->vehicleGetSpeed(vID);
        linkStatus = TraCI->vehicleGetTLLinkStatus(vID);
    }
    // CACC vehicle stream
    else if(vehicleDataLevel == 4)
    {
        timeStep = (simTime()-updateInterval).dbl();
        pos = TraCI->vehicleGetLanePosition(vID);
        speed = TraCI->vehicleGetSpeed(vID);
        accel = TraCI->vehicleGetCurrentAccel(vID);

        // get the space gap
        std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(vID, 900);
        std::string vleaderID = vleaderIDnew[0];
        spaceGap = (vleaderID != "") ? atof(vleaderIDnew[1].c_str()) : -1;
    }

    VehicleData *tmp = new VehicleData(timeStep, vID.c_str(), vType.c_str(),
            lane.c_str(), pos, speed, accel,
            CFMode.c_str(), timeGapSetting, spaceGap, timeGap,
            TLid.c_str(), linkStatus);

    Vec_vehiclesData.push_back(*tmp);
}


void Statistics::vehiclesDataToFile()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/vehicleData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;

        if(vehicleDataLevel == 2)
        {
            int TLMode = (*router->net->TLs.begin()).second->TLLogicMode;
            std::ostringstream filePrefix;
            filePrefix << router->totalVehicleCount << "_" << router->nonReroutingVehiclePercent << "_" << TLMode;
            fileName << filePrefix.str() << "_vehicleData.txt";
        }
        else
        {
            fileName << std::setfill('0') << std::setw(3) << currentRun << "_vehicleData.txt";
        }

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

    double oldTime = -1;
    int index = 0;

    for(auto &y : Vec_vehiclesData)
    {
        // write header only once
        if(oldTime == -1)
        {
            fprintf (filePtr, "%-10s","index");
            fprintf (filePtr, "%-12s","timeStep");
            fprintf (filePtr, "%-20s","vehicleName");
            if(y.vehicleType != "n/a") fprintf (filePtr, "%-17s","vehicleType");
            if(y.lane != "n/a") fprintf (filePtr, "%-12s","lane");
            if(y.pos != -1) fprintf (filePtr, "%-11s","pos");
            if(y.speed != -1) fprintf (filePtr, "%-12s","speed");
            if(y.accel != std::numeric_limits<double>::infinity()) fprintf (filePtr, "%-12s","accel");
            if(y.CFMode != "n/a") fprintf (filePtr, "%-20s","CFMode");
            if(y.timeGapSetting != -1) fprintf (filePtr, "%-20s","timeGapSetting");
            if(y.spaceGap != -2) fprintf (filePtr, "%-10s","SpaceGap");
            if(y.timeGap != -2) fprintf (filePtr, "%-16s","timeGap");
            if(y.TLid != "n/a") fprintf (filePtr, "%-17s","TLid");
            if(y.linkStatus != '\0') fprintf (filePtr, "%-17s","linkStatus");

            fprintf (filePtr, "\n\n");
        }

        if(oldTime != y.time)
        {
            fprintf(filePtr, "\n");
            oldTime = y.time;
            index++;
        }

        fprintf (filePtr, "%-10d ", index);
        fprintf (filePtr, "%-10.2f ", y.time );
        fprintf (filePtr, "%-20s ", y.vehicleName.c_str());
        if(y.vehicleType != "n/a") fprintf (filePtr, "%-15s ", y.vehicleType.c_str());
        if(y.lane != "n/a") fprintf (filePtr, "%-12s ", y.lane.c_str());
        if(y.pos != -1) fprintf (filePtr, "%-10.2f ", y.pos);
        if(y.speed != -1) fprintf (filePtr, "%-10.2f ", y.speed);
        if(y.accel != std::numeric_limits<double>::infinity()) fprintf (filePtr, "%-10.2f ", y.accel);
        if(y.CFMode != "n/a") fprintf (filePtr, "%-20s", y.CFMode.c_str());
        if(y.timeGapSetting != -1) fprintf (filePtr, "%-20.2f ", y.timeGapSetting);
        if(y.spaceGap != -2) fprintf (filePtr, "%-10.2f ", y.spaceGap);
        if(y.timeGap != -2) fprintf (filePtr, "%-16.2f ", y.timeGap);
        if(y.TLid != "n/a") fprintf (filePtr, "%-17s ", y.TLid.c_str());
        if(y.linkStatus != '\0') fprintf (filePtr, "%-17c", y.linkStatus);

        fprintf (filePtr, "\n");
    }

    fclose(filePtr);
}


void Statistics::MAClayerToFile()
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
        fprintf (filePtr, "%-20ld\n ", y.MacStatsVec[10]);
    }

    fclose(filePtr);
}


void Statistics::plnManageToFile()
{
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
