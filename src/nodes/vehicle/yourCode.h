/****************************************************************************/
/// @file    yourCode.h
/// @author
/// @author  second author name
/// @date    December 2017
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

#ifndef ApplVYOURCODE_H
#define ApplVYOURCODE_H

#include "nodes/vehicle/05_PlatoonMg.h"
#include "msg/dataMsg_m.h"

namespace VENTOS {

class ApplVYourCode : public ApplVPlatoonMg
{
private:
    typedef ApplVPlatoonMg super;

    bool printCtrlData = false;
    bool sendingData = false;

public:
    ~ApplVYourCode();
    virtual void initialize(int stage);
    virtual void finish();

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    void onBeaconVehicle(BeaconVehicle* wsm);
    void onBeaconRSU(BeaconRSU* wsm);
    void onDataMsg(dataMsg *wsm);

private:
    dataMsg* generateData();
    void printControlInfo(Veins::WaveShortMessage *wsm);
};

}

#endif
