/****************************************************************************/
/// @file    AID.h
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

#ifndef ApplVAID_H
#define ApplVAID_H

#include "nodes/vehicle/02_DynamicRouting.h"
#include "msg/PlatoonMsg_m.h"

namespace VENTOS {

class ApplV_AID : public ApplVDynamicRouting
{
private:
    typedef ApplVDynamicRouting super;

protected:
    // NED
    bool AID;

    // class variables
    std::string fromLane;
    std::string toLane;
    double fromX;
    double toX;

    std::deque<std::string> laneChanges;

public:
    ~ApplV_AID();
    virtual void initialize(int stage);
    virtual void finish();

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconRSU(BeaconRSU*);

    LaneChangeMsg* prepareData(std::string, std::deque<std::string>);
};

}

#endif
