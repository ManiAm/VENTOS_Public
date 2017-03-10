/****************************************************************************/
/// @file    TraCIRecordStat.cc
/// @author  Mani Amoozadeh   <maniam@ucdavis.edu>
/// @date    Feb 2017
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

#include <cmath>
#include <algorithm>
#include <iomanip>

#undef ev
#include "boost/filesystem.hpp"
#include <boost/algorithm/string.hpp>

#include "traci/TraCIRecordStat.h"
#include "logging/VENTOS_logging.h"

namespace VENTOS {

Define_Module(VENTOS::TraCI_RecordStat);

TraCI_RecordStat::TraCI_RecordStat()
{

}


void TraCI_RecordStat::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 1)
    {
        record_sim_stat = par("record_sim_stat").boolValue();
    }
}


void TraCI_RecordStat::finish()
{
    super::finish();

    // record simulation data one last time before closing TraCI
    if(!TraCIclosed && record_sim_stat)
        record_Sim_data();

    save_Sim_data_toFile();
    save_Veh_data_toFile();
}


void TraCI_RecordStat::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void TraCI_RecordStat::init_Sim_data()
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


void TraCI_RecordStat::record_Sim_data()
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
            entry.loaded = simulationGetLoadedVehiclesCount();
        else if(record == "departed")
            entry.departed = departedVehicleCount;
        else if(record == "arrived")
            entry.arrived = arrivedVehicleCount;
        else if(record == "running")
            entry.running = activeVehicleCount;
        else if(record == "waiting")
            entry.waiting = simulationGetMinExpectedNumber() - activeVehicleCount;
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name", record.c_str());
    }

    sim_record_status.push_back(entry);
}


void TraCI_RecordStat::save_Sim_data_toFile()
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
        fprintf (filePtr, "sim timeStep    %u ms\n", simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", simulationGetDuration().c_str());
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


void TraCI_RecordStat::init_Veh_data(std::string SUMOID, omnetpp::cModule *mod)
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
        std::vector<std::string> record_tokenize;
        for(std::string appl : records)
        {
            if(appl == "")
                throw omnetpp::cRuntimeError("Invalid record_list format in vehicle '%s'", SUMOID.c_str());

            // remove leading and trailing spaces from the string
            boost::trim(appl);

            // convert to lower case
            boost::algorithm::to_lower(appl);

            record_tokenize.push_back(appl);
        }

        veh_status_entry_t entry = {active, record_tokenize};
        record_status[SUMOID] = entry;
    }
}


// is called in each time step for each vehicle
void TraCI_RecordStat::record_Veh_data(std::string vID, bool arrived)
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
            entry.vehType = vehicleGetTypeID(vID);
        else if(record == "lane")
            entry.lane = vehicleGetLaneID(vID);
        else if(record == "lanepos")
            entry.lanePos = vehicleGetLanePosition(vID);
        else if(record == "speed")
            entry.speed = vehicleGetSpeed(vID);
        else if(record == "accel")
            entry.accel = vehicleGetCurrentAccel(vID);
        else if(record == "departure")
            entry.departure = vehicleGetDepartureTime(vID);
        else if(record == "arrival")
            entry.arrival = vehicleGetArrivalTime(vID);
        else if(record == "route")
        {
            // convert vector of string to std::string
            std::string route_edges = "' ";
            for (auto &s : vehicleGetRoute(vID)) { route_edges = route_edges + s + " "; }
            route_edges += "'";

            entry.route = route_edges;
        }
        else if(record == "routeduration")
        {
            double departure = vehicleGetDepartureTime(vID);
            if(departure != -1)
                entry.routeDuration = (omnetpp::simTime() - departure - updateInterval).dbl();
        }
        else if(record == "drivingdistance")
            entry.drivingDistance = vehicleGetDrivingDistance(vID);
        else if(record == "cfmode")
        {
            CFMODES_t CFMode_Enum = vehicleGetCarFollowingMode(vID);
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
            entry.timeGapSetting = vehicleGetTimeGap(vID);
        else if(record == "timegap")
        {
            double speed = (entry.speed != -1) ? entry.speed : vehicleGetSpeed(vID);

            auto leader = vehicleGetLeader(vID, 900);
            double spaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;

            // calculate timeGap (if leading is present)
            if(leader.leaderID != "" && speed != 0)
                entry.timeGap = spaceGap / speed;
            else
                entry.timeGap = -1;
        }
        else if(record == "frontspacegap")
        {
            auto leader = vehicleGetLeader(vID, 900);
            entry.frontSpaceGap = (leader.leaderID != "") ? leader.distance2Leader : -1;
        }
        else if(record == "rearspacegap")
        {

        }
        else if(record == "nexttlid")
        {
            std::vector<TL_info_t> res = vehicleGetNextTLS(vID);

            if(!res.empty())
                entry.nextTLId = res[0].TLS_id;
            else
                entry.nextTLId = "";
        }
        else if(record == "nexttllinkstat")
        {
            std::vector<TL_info_t> res = vehicleGetNextTLS(vID);

            if(!res.empty())
                entry.nextTLLinkStat = res[0].linkState;
            else
                entry.nextTLLinkStat = 'n';
        }
        else
            throw omnetpp::cRuntimeError("'%s' is not a valid record name in veh '%s'", record.c_str(), vID.c_str());

        auto it = allColumns.find(record);
        if(it == allColumns.end())
        {
            allColumns[record] = columnNumber;
            columnNumber++;
        }
    }

    collected_veh_data.push_back(entry);
}


void TraCI_RecordStat::save_Veh_data_toFile()
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
        fprintf (filePtr, "sim timeStep    %u ms\n", simulationGetTimeStep());
        fprintf (filePtr, "startDateTime   %s\n", simulationGetStartTime().c_str());
        fprintf (filePtr, "endDateTime     %s\n", simulationGetEndTime().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", simulationGetDuration().c_str());
    }

    std::string columns_sorted[allColumns.size()];
    for(auto it : allColumns)
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

}
