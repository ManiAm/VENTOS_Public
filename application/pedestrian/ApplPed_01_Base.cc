/****************************************************************************/
/// @file    ApplPed_01_Base.cc
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

#include "ApplPed_01_Base.h"

namespace VENTOS {

const simsignalwrap_t ApplPedBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplPedBase);

ApplPedBase::~ApplPedBase()
{

}

void ApplPedBase::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);

		TraCI_Mobility = TraCIMobilityAccess().get(getParentModule());
        TraCI = FindModule<TraCI_Extend*>::findGlobalModule();

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        headerLength = par("headerLength").longValue();

        // pedestrian id in omnet++
		myId = getParentModule()->getIndex();

		// pedestrian full id in omnet++
		myFullId = getParentModule()->getFullName();

        // pedestrian id in sumo
        SUMOpID = TraCI_Mobility->getExternalId();

        // pedestrian type in sumo
        SUMOpType = TraCI->personGetTypeID(SUMOpID);

        // store the time of entry
        entryTime = simTime().dbl();

        // findHost()->subscribe(mobilityStateChangedSignal, this);
	}
}


void ApplPedBase::finish()
{
    // findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplPedBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplPedBase::handleSelfMsg(cMessage* msg)
{

}


void ApplPedBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

}

