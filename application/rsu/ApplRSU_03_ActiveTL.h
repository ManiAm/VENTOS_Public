/****************************************************************************/
/// @file    ApplRSU_03_ActiveTL.h
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

#ifndef APPLRSUTLVANET_H_
#define APPLRSUTLVANET_H_

#include "ApplRSU_02_Monitor.h"

namespace VENTOS {

class queuedVehiclesEntry
{
public:
    std::string vehicleType;
    double entryTime;
    double entrySpeed;

    queuedVehiclesEntry(std::string str1, double d1, double d2)
    {
        this->vehicleType = str1;
        this->entryTime = d1;
        this->entrySpeed = d2;
    }
};


class laneInfoEntry
{
public:
    std::string TLid;
    double firstDetectedTime;
    double lastDetectedTime;
    double passageTime;
    int totalVehCount;
    std::map<std::string /*vehicle id*/, queuedVehiclesEntry> queuedVehicles;

    laneInfoEntry(std::string str1, double d1, double d2, double d3, int i1, std::map<std::string, queuedVehiclesEntry> mapV)
    {
        this->TLid = str1;
        this->firstDetectedTime = d1;
        this->lastDetectedTime = d2;
        this->passageTime = d3;
        this->totalVehCount = i1;
        this->queuedVehicles = mapV;
    }
};


class ApplRSUTLVANET : public ApplRSUMonitor
{
public:
    ~ApplRSUTLVANET();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleSelfMsg(cMessage* msg);

protected:
    void virtual executeEachTimeStep(bool);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);
    virtual void onData(LaneChangeMsg*);

    void UpdateLaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed);
    void UpdateLaneInfoRemove(std::string counter, std::string sender);

public:
    std::map<std::string /*lane*/, laneInfoEntry> laneInfo;   // collected info per lane by this RSU. Note that each RSU has
                                                              // a local copy of laneInfo that contains the lane info for this specific TL
};

}

#endif
