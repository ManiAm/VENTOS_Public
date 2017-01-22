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

#include "rsu/01_Base.h"
#include "msg/BeaconVehicle_m.h"
#include "msg/BeaconBicycle_m.h"
#include "msg/BeaconPedestrian_m.h"
#include "msg/BeaconRSU_m.h"
#include "msg/LaneChangeMsg_m.h"

namespace VENTOS {

class detectedVehicleEntry
{
public:
    std::string vehicleName;
    std::string vehicleType;
    std::string lane;
    Coord pos;
    std::string TLid;
    double entryTime;
    double leaveTime;
    double entrySpeed;

    detectedVehicleEntry(std::string str1, std::string str2="", std::string str3="", Coord xy=Coord(), std::string str4="", double entryT=-1, double leaveT=-1, double entryS=-1)
    {
        this->vehicleName = str1;
        this->vehicleType = str2;
        this->lane = str3;
        this->pos = xy;
        this->TLid = str4;
        this->entryTime = entryT;
        this->leaveTime = leaveT;
        this->entrySpeed = entryS;
    }

    // overload == for search
    friend bool operator== (const detectedVehicleEntry &v1, const detectedVehicleEntry &v2) {
        return ( v1.vehicleName == v2.vehicleName );
    }

    // overload < for sort
    friend bool operator < (const detectedVehicleEntry &v1, const detectedVehicleEntry &v2) {
        return (v1.vehicleType < v2.vehicleType);
    }
};

typedef struct allVehiclesEntry
{
    std::string vehType;
    int vehStatus;    // driving, waiting ?
    double entryTime;
    double entrySpeed;
} allVehiclesEntry_t;

typedef struct laneInfoEntry
{
    std::string TLid;
    double firstDetectedTime;
    double lastDetectedTime;
    double passageTime;
    int totalVehCount;
    std::map<std::string /*vehicle id*/, allVehiclesEntry> allVehicles;
} laneInfoEntry_t;


class ApplRSUMonitor : public ApplRSUBase
{
public:
    // collected info per lane by this RSU. Note that each RSU has
    // a local copy of laneInfo that contains the lane info for this specific TL
    std::map<std::string /*lane*/, laneInfoEntry_t> laneInfo;

private:
    typedef ApplRSUBase super;

    bool activeDetection;
    bool collectVehApproach;

    // all incoming lanes for the intersection that this RSU belongs to
    std::map<std::string /*lane*/, std::string /*TLid*/> lanesTL;

    // keeping track of detected vehicles --used by all RSUs
    static std::vector<detectedVehicleEntry> Vec_detectedVehicles;

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
    template <typename T> void onBeaconAny(T wsm);

    void LaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed);
    void LaneInfoUpdate(std::string lane, std::string sender, std::string senderType, double speed);
    void LaneInfoRemove(std::string counter, std::string sender);

    void save_VehApproach_toFile();
};

}

#endif
