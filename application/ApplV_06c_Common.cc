
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
            TraCI->commandSetTg(SUMOvID, wsm->getDblValue());
        }
    }
}

}

