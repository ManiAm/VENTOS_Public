/****************************************************************************/
/// @file    yourCode.cc
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

#include "nodes/rsu/yourCode.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUYourCode);

ApplRSUYourCode::~ApplRSUYourCode()
{

}


void ApplRSUYourCode::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {

    }
}


void ApplRSUYourCode::finish()
{
    super::finish();
}


void ApplRSUYourCode::executeEachTimeStep()
{
    // call the super method
    super::executeEachTimeStep();
}


void ApplRSUYourCode::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplRSUYourCode::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    super::onBeaconVehicle(wsm);
}


void ApplRSUYourCode::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    super::onBeaconRSU(wsm);
}


void ApplRSUYourCode::onDataMsg(dataMsg *wsm)
{
    // do not pass it down!
}

}
