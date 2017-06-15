/****************************************************************************/
/// @file    04_Platoon.h
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

#ifndef APPLVPLATOONFORMED_H
#define APPLVPLATOONFORMED_H

#include "nodes/vehicle/03_AID.h"

namespace VENTOS {

class ApplVPlatoon : public ApplV_AID
{
protected:
    int plnMode = -1;
    std::string myPlnID = "";
    int myPlnDepth = -1;

    // only the platoon leader know platoon size and member list
    int plnSize = -1;
    std::deque<std::string> plnMembersList;

    bool record_platoon_stat = false;
    omnetpp::cMessage* platoonMonitorTIMER = NULL;
    double updateInterval = -1;

public:
    enum platooningMode
    {
        platoonOff = 1,
        platoonNoManagement = 2,
        platoonManagement = 3,
        platoonMAX,
    };

private:
    typedef ApplV_AID super;

public:
    ~ApplVPlatoon();
    virtual void initialize(int stage);
    virtual void finish();

    int getPlatoonMode() {return plnMode;}
    std::string getPlatoonId() {return myPlnID;}
    int getPlatoonDepth() {return myPlnDepth;}
    int getPlatoonSize() {return plnSize;}
    std::deque<std::string> getPlatoonMembers() {return plnMembersList;};

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconRSU(BeaconRSU*);
};

}

#endif
