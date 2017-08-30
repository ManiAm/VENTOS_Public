/****************************************************************************/
/// @file    Monitor.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Dec 2015
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

#ifndef APPLRSUMONITOR_H_
#define APPLRSUMONITOR_H_

#include "nodes/rsu/01_Beacon.h"
#include "msg/BeaconVehicle_m.h"
#include "msg/BeaconBicycle_m.h"
#include "msg/BeaconPedestrian_m.h"
#include "msg/BeaconRSU_m.h"
#include "msg/LaneChangeMsg_m.h"

namespace VENTOS {

typedef struct vehicleEntryRSU
{
    std::string vehType;
    double entryTime;
    double entrySpeed;
    double currentSpeed;
} vehicleEntryRSU_t;

typedef struct laneInfoEntry
{
    std::string TLid;
    double firstDetectedTime;
    double lastDetectedTime;
    double passageTime;
    int totalVehCount;
    std::map<std::string /*vehicle id*/, vehicleEntryRSU_t> allVehicles;
} laneInfoEntry_t;


class ApplRSUMonitor : public ApplRSUBeacon
{
public:
    // collected info per lane by this RSU (only in the current simulation time).
    // Note that each RSU has a local copy of laneInfo
    typedef std::map<std::string /*lane*/, laneInfoEntry_t> laneInfo_t;
    laneInfo_t laneInfo;

private:
    typedef ApplRSUBeacon super;

    bool record_vehApproach_stat;

    // all incoming lanes for the intersection that this RSU belongs to
    std::map<std::string /*lane*/, std::string /*TLid*/> lanesTL;

    static std::vector<std::string> junctionList;

    typedef struct vehApproachEntry
    {
        std::string TLid;
        std::string vehicleName;
        std::string vehicleType;
        std::string entryLane;
        TraCICoord entryPos;
        double entrySpeed;
        double entryTime;
        double leaveTime;
    } vehApproachEntry_t;

    // keeping track of all approaching vehicles
    std::vector<vehApproachEntry_t> vehApproach;

    typedef struct vehApproachPerLaneEntry
    {
        double time;
        laneInfo_t laneInfo;
    } vehApproachPerLaneEntry_t;

    // keeping track of all approaching vehicles per lane
    std::vector<vehApproachPerLaneEntry_t> vehApproachPerLane;

public:
    ~ApplRSUMonitor();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleSelfMsg(omnetpp::cMessage* msg);

protected:
    void virtual executeEachTimeStep();

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);

private:
    void initVariables();
    void check_RSU_pos();

    template <typename T> void onBeaconAny(T wsm);

    void LaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed);
    void LaneInfoUpdate(std::string lane, std::string sender, double speed);
    void LaneInfoRemove(std::string counter, std::string sender);

    void save_VehApproach_toFile();
    void save_VehApproachPerLane_toFile();
};

}

#endif
