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
    int plnMode;
    std::string myPlnID = "";
    int myPlnDepth = -1;
    int plnSize = -1;

    enum platooningMode
    {
        platoonOff = 1,
        platoonNoManagement = 2,
        platoonManagement = 3,
    };

private:
    typedef ApplV_AID super;

public:
    ~ApplVPlatoon();
    virtual void initialize(int stage);
    virtual void finish();

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconRSU(BeaconRSU*);

    std::string getPlatoonId() {return myPlnID;}
    int getPlatoonDepth() {return myPlnDepth;}
    int getPlatoonSize() {return plnSize;}
};

}

#endif
