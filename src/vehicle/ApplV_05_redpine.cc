/****************************************************************************/
/// @file    ApplV_redpine.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jun 2016
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

#include "ApplV_05_redpine.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVRedpine);

ApplVRedpine::~ApplVRedpine()
{

}


void ApplVRedpine::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        EEBL = par("EEBL").boolValue();

    }
}


void ApplVRedpine::finish()
{
    super::finish();

    //findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplVRedpine::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


// is called, every time the position of vehicle changes
void ApplVRedpine::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);

    ChannelMobilityPtrType const mobility = omnetpp::check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


void ApplVRedpine::receiveDataFromBoard(redpineData* data)
{
    std::cout << SUMOID << " received a HIL message: \n";
    std::cout << data->getSrcPort() << "\n";
    std::cout.flush();

}


void ApplVRedpine::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    super::onBeaconVehicle(wsm);
}


void ApplVRedpine::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    super::onBeaconRSU(wsm);
}


void ApplVRedpine::onData(PlatoonMsg* wsm)
{
    // pass it down
    super::onData(wsm);
}

}
