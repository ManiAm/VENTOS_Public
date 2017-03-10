/****************************************************************************/
/// @file    TraCIRecordStat.h
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

#ifndef TRACIRECORDSTAT_H
#define TRACIRECORDSTAT_H

#include <queue>
#include "traci/TraCICommands.h"

namespace VENTOS {

class TraCI_RecordStat :  public TraCI_Commands
{
private:
    typedef TraCI_Commands super;

    typedef struct sim_status_entry
    {
        double timeStep;
        long int loaded;
        long int departed;
        long int arrived;
        long int running;
        long int waiting;
    } sim_status_entry_t;

    bool record_sim_stat;
    std::vector<std::string> record_sim_tokenize;
    std::vector<sim_status_entry_t> sim_record_status;

    typedef struct veh_status_entry
    {
        bool active;  // should we record statistics for this vehicle?
        std::vector<std::string> record_list;  // type of data we need to record for this vehicle
    } veh_status_entry_t;

    std::map<std::string /*SUMO id*/, veh_status_entry_t> record_status;

    typedef struct veh_data_entry
    {
        double timeStep;
        std::string vehId;
        std::string vehType;
        std::string lane;
        double lanePos;
        double speed;
        double accel;
        double departure;
        double arrival;
        std::string route;
        double routeDuration;
        double drivingDistance;
        std::string CFMode;
        double timeGapSetting;
        double timeGap;
        double frontSpaceGap;
        double rearSpaceGap;
        std::string nextTLId;  // TLid that controls this vehicle. Empty string means the vehicle is not controlled by any TLid
        char nextTLLinkStat;   // status of the TL ahead (character 'n' means no TL ahead)
    } veh_data_entry_t;

    std::vector<veh_data_entry_t> collected_veh_data;
    std::map<std::string, int /*order*/> allColumns;

protected:
    uint32_t departedVehicleCount = 0; // accumulated number of departed vehicles
    uint32_t arrivedVehicleCount = 0;  // accumulated number of arrived vehicles

    uint32_t activeVehicleCount = 0;  // number of active vehicles (be it parking or driving) at current time step
    uint32_t parkingVehicleCount = 0; // number of parking vehicles at current time step
    uint32_t drivingVehicleCount = 0; // number of driving vehicles at current time step

public:
    TraCI_RecordStat();

    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(omnetpp::cMessage *msg);

protected:
    void init_Sim_data();
    void record_Sim_data();
    void save_Sim_data_toFile();

    void init_Veh_data(std::string SUMOID, omnetpp::cModule *mod);
    void record_Veh_data(std::string vID, bool arrived = false);
    void save_Veh_data_toFile();
};

}

#endif
