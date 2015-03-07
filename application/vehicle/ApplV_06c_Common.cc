/****************************************************************************/
/// @file    ApplV_06c_Common.cc
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

#include "ApplV_06_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::common_handleSelfMsg(cMessage* msg)
{


}


void ApplVPlatoonMg::common_BeaconFSM(BeaconVehicle* wsm)
{


}


void ApplVPlatoonMg::common_DataFSM(PlatoonMsg* wsm)
{
    if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getType() == CHANGE_PL && wsm->getSender() == plnID )
        {
            if( string(wsm->getRecipient()) == "multicast" || wsm->getRecipient() == SUMOvID )
            {
                // send ACK
                PlatoonMsg* dataMsg = prepareData(wsm->getSender(), ACK, wsm->getSendingPlatoonID());
                EV << "### " << SUMOvID << ": sent ACK." << endl;
                printDataContent(dataMsg);
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);

                // save my current platoon ID
                oldPlnID = plnID;

                // these should be updated after sending ACK!
                plnID = wsm->getStrValue();
                myPlnDepth = myPlnDepth + wsm->getDblValue();
            }
        }
        // if my old platoon leader asks me to change my leader to the current one
        // I have done it before, so I only ACK
        else if( wsm->getType() == CHANGE_PL && wsm->getSender() == oldPlnID && wsm->getStrValue() == plnID )
        {
            if( string(wsm->getRecipient()) == "multicast" || wsm->getRecipient() == SUMOvID )
            {
                // send ACK
                PlatoonMsg* dataMsg = prepareData(wsm->getSender(), ACK, wsm->getSendingPlatoonID());
                EV << "### " << SUMOvID << ": sent ACK." << endl;
                printDataContent(dataMsg);
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);
            }
        }
        else if(wsm->getType() == CHANGE_Tg && wsm->getSender() == plnID)
        {
            TraCI->commandSetVehicleTg(SUMOvID, wsm->getDblValue());
        }
    }
}

}

