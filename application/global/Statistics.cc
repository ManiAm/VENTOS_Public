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
#include <ApplRSU_04_Manager.h>
#include "Router.h"

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
        terminate = module->par("terminate").doubleValue();
        updateInterval = module->par("updateInterval").doubleValue();

        cModule *rmodule = simulation.getSystemModule()->getSubmodule("router");
        router = static_cast< Router* >(rmodule);

        collectVehiclesData = par("collectVehiclesData").boolValue();
        useDetailedFilenames = par("useDetailedFilenames").boolValue();

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

}


void Statistics::handleMessage(cMessage *msg)
{

}


void Statistics::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        Statistics::executeEachTimestep(i);
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
        std::vector<MacStatEntry>::iterator counter = std::find(Vec_MacStat.begin(), Vec_MacStat.end(), *searchFor);

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


void Statistics::executeEachTimestep(bool simulationDone)
{
    if(collectVehiclesData)
    {
        vehiclesData();   // collecting data from all vehicles in each timeStep

        if(ev.isGUI()) vehiclesDataToFile();   // (if in GUI) write what we have collected so far
        else if(simulationDone) vehiclesDataToFile();  // (if in CMD) write to file at the end of simulation
    }

    if(reportMAClayerData)
    {
        if(ev.isGUI()) MAClayerToFile();    // (if in GUI) write what we have collected so far
        else if(simulationDone) MAClayerToFile();  // (if in CMD) write to file at the end of simulation
    }

    if(reportPlnManagerData)
    {
        // (if in GUI) write what we have collected so far
        if(ev.isGUI())
        {
            plnManageToFile();
            plnStatToFile();
        }
        else if(simulationDone)
        {
            plnManageToFile();
            plnStatToFile();
        }
    }

    if(reportBeaconsData)
    {
        if(ev.isGUI()) beaconToFile();    // (if in GUI) write what we have collected so far
        else if(simulationDone) beaconToFile();  // (if in CMD) write to file at the end of simulation
    }
}


void Statistics::vehiclesData()
{
    for(std::list<std::string>::iterator i = lanesList.begin(); i != lanesList.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> allVeh = TraCI->laneGetLastStepVehicleIDs( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = allVeh.rbegin(); k != allVeh.rend(); ++k)
            saveVehicleData(k->c_str());
    }
}


void Statistics::saveVehicleData(std::string vID)
{
    double timeStep = (simTime()-updateInterval).dbl();
    std::string vType = TraCI->vehicleGetTypeID(vID);
    std::string lane = TraCI->vehicleGetLaneID(vID);
    double pos = TraCI->vehicleGetLanePosition(vID);
    double speed = TraCI->vehicleGetSpeed(vID);
    double accel = TraCI->vehicleGetCurrentAccel(vID);

    if(useDetailedFilenames)
    {
        VehicleData *tmp = new VehicleData(timeStep, vID.c_str(), vType.c_str(),
                lane.c_str(), pos, speed, accel,
                "",-1, -1, -1, "", -1);
        Vec_vehiclesData.push_back(*tmp);
    }
    else
    {
        int CFMode_Enum = TraCI->vehicleGetCarFollowingMode(vID);
        std::string CFMode;

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
        double timeGapSetting = TraCI->vehicleGetTimeGap(vID);

        // get the gap
        std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(vID, 900);
        std::string vleaderID = vleaderIDnew[0];
        double spaceGap = -1;

        if(vleaderID != "")
            spaceGap = atof( vleaderIDnew[1].c_str() );

        // calculate timeGap (if leading is present)
        double timeGap = -1;

        if(vleaderID != "" && speed != 0)
            timeGap = spaceGap / speed;

        // get the TLid that controls this vehicle
        // empty string means the vehicle is not controlled by any TLid
        // it is disabled for now to make the simulation faster
        std::string TLid = "";   // TraCI->vehicleGetTLID(vID);

        // get the signal status ahead
        // character 'n' means no link status
        char linkStatus = TraCI->vehicleGetTLLinkStatus(vID);

        VehicleData *tmp = new VehicleData(timeStep, vID.c_str(), vType.c_str(),
                lane.c_str(), pos, speed, accel,
                CFMode.c_str(), timeGapSetting, spaceGap, timeGap,
                TLid.c_str(), linkStatus);
        Vec_vehiclesData.push_back(*tmp);
    }
}


void Statistics::vehiclesDataToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/vehicleData.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;

        if(useDetailedFilenames)
        {
            int TLMode = (*router->net->TLs.begin()).second->TLLogicMode;
            std::ostringstream filePrefix;
            filePrefix << router->totalVehicleCount << "_" << router->nonReroutingVehiclePercent << "_" << TLMode;
            fileName << filePrefix.str() << "_vehicleData.txt";
        }
        else
        {
            fileName << currentRun << "_vehicleData.txt";
        }

        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    if(useDetailedFilenames)
    {
        // write header
        fprintf (filePtr, "%-10s","index");
        fprintf (filePtr, "%-12s","timeStep");
        fprintf (filePtr, "%-15s","vehicleName");
        fprintf (filePtr, "%-17s","vehicleType");
        fprintf (filePtr, "%-12s","lane");
        fprintf (filePtr, "%-11s","pos");
        fprintf (filePtr, "%-12s","speed");
        fprintf (filePtr, "%-12s\n\n","accel");

        double oldTime = -1;
        int index = 0;

        // write body
        for(std::vector<VehicleData>::iterator y = Vec_vehiclesData.begin(); y != Vec_vehiclesData.end(); ++y)
        {
            if(oldTime != y->time)
            {
                fprintf(filePtr, "\n");
                oldTime = y->time;
                index++;
            }

            fprintf (filePtr, "%-10d ", index);
            fprintf (filePtr, "%-10.2f ", y->time );
            fprintf (filePtr, "%-15s ", y->vehicleName.c_str());
            fprintf (filePtr, "%-15s ", y->vehicleType.c_str());
            fprintf (filePtr, "%-12s ", y->lane.c_str());
            fprintf (filePtr, "%-10.2f ", y->pos);
            fprintf (filePtr, "%-10.2f ", y->speed);
            fprintf (filePtr, "%-10.2f \n", y->accel);
        }
    }
    else
    {
        // write header
        fprintf (filePtr, "%-10s","index");
        fprintf (filePtr, "%-12s","timeStep");
        fprintf (filePtr, "%-15s","vehicleName");
        fprintf (filePtr, "%-17s","vehicleType");
        fprintf (filePtr, "%-12s","lane");
        fprintf (filePtr, "%-11s","pos");
        fprintf (filePtr, "%-12s","speed");
        fprintf (filePtr, "%-12s","accel");
        fprintf (filePtr, "%-20s","CFMode");
        fprintf (filePtr, "%-20s","timeGapSetting");
        fprintf (filePtr, "%-10s","SpaceGap");
        fprintf (filePtr, "%-16s","timeGap");
        fprintf (filePtr, "%-17s","TLid");
        fprintf (filePtr, "%-17s\n\n","linkStatus");

        double oldTime = -1;
        int index = 0;

        // write body
        for(std::vector<VehicleData>::iterator y = Vec_vehiclesData.begin(); y != Vec_vehiclesData.end(); ++y)
        {
            if(oldTime != y->time)
            {
                fprintf(filePtr, "\n");
                oldTime = y->time;
                index++;
            }

            fprintf (filePtr, "%-10d ", index);
            fprintf (filePtr, "%-10.2f ", y->time );
            fprintf (filePtr, "%-15s ", y->vehicleName.c_str());
            fprintf (filePtr, "%-15s ", y->vehicleType.c_str());
            fprintf (filePtr, "%-12s ", y->lane.c_str());
            fprintf (filePtr, "%-10.2f ", y->pos);
            fprintf (filePtr, "%-10.2f ", y->speed);
            fprintf (filePtr, "%-10.2f ", y->accel);
            fprintf (filePtr, "%-20s", y->CFMode.c_str());
            fprintf (filePtr, "%-20.2f ", y->timeGapSetting);
            fprintf (filePtr, "%-10.2f ", y->spaceGap);
            fprintf (filePtr, "%-16.2f ", y->timeGap);
            fprintf (filePtr, "%-17s ", y->TLid.c_str());
            fprintf (filePtr, "%-17c \n", y->linkStatus);
        }
    }

    fclose(filePtr);
}


void Statistics::MAClayerToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/MACdata.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_MACdata.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

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
    fprintf (filePtr, "%-20s","ReceivedBroadcasts\n\n");

    // write body
    for(std::vector<MacStatEntry>::iterator y = Vec_MacStat.begin(); y != Vec_MacStat.end(); ++y)
    {
        fprintf (filePtr, "%-20.2f ", y->time);
        fprintf (filePtr, "%-20s ", y->name.c_str());
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[0]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[1]);
        fprintf (filePtr, "%-30ld ", y->MacStatsVec[2]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[3]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[4]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[5]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[6]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[7]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[8]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[9]);
        fprintf (filePtr, "%-20ld ", y->MacStatsVec[10]);

        fprintf (filePtr, "\n" );
    }

    fclose(filePtr);
}


void Statistics::plnManageToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/plnManage.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_plnManage.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

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
    for(std::vector<plnManagement>::iterator y = Vec_plnManagement.begin(); y != Vec_plnManagement.end(); ++y)
    {
        // make the log more readable :)
        if(y->sender != oldSender || y->time != oldTime)
        {
            fprintf(filePtr, "\n");
            oldSender = y->sender;
            oldTime = y->time;
        }

        fprintf (filePtr, "%-10.2f ", y->time);
        fprintf (filePtr, "%-15s ", y->sender.c_str());
        fprintf (filePtr, "%-17s ", y->receiver.c_str());
        fprintf (filePtr, "%-30s ", y->type.c_str());
        fprintf (filePtr, "%-18s ", y->sendingPlnID.c_str());
        fprintf (filePtr, "%-20s\n", y->receivingPlnID.c_str());
    }

    fclose(filePtr);
}


void Statistics::plnStatToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/plnStat.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_plnStat.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from platoon");
    fprintf (filePtr, "%-20s","to platoon");
    fprintf (filePtr, "%-20s\n\n","comment");

    std::string oldPln = "";

    // write body
    for(std::vector<plnStat>::iterator y = Vec_plnStat.begin(); y != Vec_plnStat.end(); ++y)
    {
        if(y->from != oldPln)
        {
            fprintf(filePtr, "\n");
            oldPln = y->from;
        }

        fprintf (filePtr, "%-10.2f ", y->time);
        fprintf (filePtr, "%-20s ", y->from.c_str());
        fprintf (filePtr, "%-20s ", y->to.c_str());
        fprintf (filePtr, "%-20s\n", y->maneuver.c_str());
    }

    fclose(filePtr);
}


void Statistics::beaconToFile()
{
    boost::filesystem::path filePath;

    if( ev.isGUI() )
    {
        filePath = "results/gui/beaconsStat.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << currentRun << "_beaconsStat.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-12s","timeStep");
    fprintf (filePtr, "%-20s","from");
    fprintf (filePtr, "%-20s","to");
    fprintf (filePtr, "%-20s\n\n","dropped");

    for(std::vector<BeaconStat>::iterator y = Vec_Beacons.begin(); y != Vec_Beacons.end(); ++y)
    {
        fprintf (filePtr, "%-12.2f ", y->time);
        fprintf (filePtr, "%-20s ", y->senderID.c_str());
        fprintf (filePtr, "%-20s ", y->receiverID.c_str());
        fprintf (filePtr, "%-20d \n", y->dropped);
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
