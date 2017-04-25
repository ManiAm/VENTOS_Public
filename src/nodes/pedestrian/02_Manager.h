/****************************************************************************/
/// @file    Manager.h
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

#ifndef ApplPEDMANAGER_H
#define ApplPEDMANAGER_H

#include "nodes/pedestrian/01_Beacon.h"
#include "msg/BeaconVehicle_m.h"
#include "msg/BeaconRSU_m.h"
#include "msg/PlatoonMsg_m.h"

namespace VENTOS {

class ApplPedManager : public ApplPedBeacon
{
private:
    typedef ApplPedBeacon super;

public:
    ~ApplPedManager();
    virtual void initialize(int stage);
    virtual void finish();

protected:
    // Methods
    virtual void handleLowerMsg(omnetpp::cMessage*);
    virtual void handleSelfMsg(omnetpp::cMessage*);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconRSU(BeaconRSU*);
};

}

#endif
