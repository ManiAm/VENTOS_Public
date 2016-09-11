/****************************************************************************/
/// @file    ApplV_02_Beacon.h
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

#ifndef ApplVBeacon_H
#define ApplVBeacon_H

#include "ApplV_01_Base.h"
#include "BeaconVehicle_m.h"
#include <deque>

namespace VENTOS {

class ApplVBeacon : public ApplVBase
{
public:
    ~ApplVBeacon();
    virtual void initialize(int stage);
    virtual void finish();

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);

    bool isBeaconFromLeading(BeaconVehicle*);
    bool isBeaconFromMyPlatoonLeader(BeaconVehicle*);

private:
    BeaconVehicle* prepareBeacon();

private:
    typedef ApplVBase super;

protected:
    // NED
    bool DSRCenabled;
    double sonarDist;

    // NED variables (beaconing parameters)
    bool sendBeacons;
    double beaconInterval;
    double maxOffset;
    int beaconLengthBits;
    int beaconPriority;

    bool signalBeaconing;

    // NED variables (data message parameters)
    int dataLengthBits;
    bool dataOnSch;
    int dataPriority;

    // Class variables
    omnetpp::simtime_t individualOffset;
    omnetpp::cMessage* VehicleBeaconEvt;

    std::string plnID;
    int myPlnDepth;
    int plnSize;
    std::deque<std::string> plnMembersList;
};

}

#endif
