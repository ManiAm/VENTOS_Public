/****************************************************************************/
/// @file    BaseWaveApplLayer.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Feb 2017
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
// This program is distributed in the hope that it will be useful}},
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "global/BaseWaveApplLayer.h"

namespace VENTOS {

const simsignalwrap_t BaseWaveApplLayer::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);
//const simsignalwrap_t BaseWaveApplLayer::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

BaseWaveApplLayer::~BaseWaveApplLayer()
{
    cancelAndDelete(sendBeaconEvt);

    //findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void BaseWaveApplLayer::initialize(int stage)
{
    BaseApplLayer::initialize(stage);

    if (stage == 0)
    {
        // get a pointer to the TraCI module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to the mobility module
        module = this->getParentModule()->getSubmodule("mobility");
        mobility = static_cast<BaseMobility *>(module);
        ASSERT(mobility);

        // get a pointer to the statistics module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<Statistics *>(module);
        ASSERT(STAT);

        // get a pointer to the mac1609_4 module
        module = this->getParentModule()->getSubmodule("nic")->getSubmodule("mac1609_4");
        mac = static_cast<Veins::Mac1609_4 *>(module);
        ASSERT(mac);

        headerLength = par("headerLength").longValue();

        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();
        beaconInterval =  par("beaconInterval");

        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSCH = par("dataOnSCH").boolValue();
        dataPriority = par("dataPriority").longValue();

        DSRCenabled = getParentModule()->par("DSRCenabled").boolValue();
        hasOBU = getParentModule()->par("hasOBU").boolValue();
        IPaddress = getParentModule()->par("IPaddress").stringValue();

        myId = getParentModule()->getIndex();
        myFullId = getParentModule()->getFullName();
        entryTime = omnetpp::simTime().dbl();

        SUMOID = getParentModule()->par("SUMOID").stringValue();
        SUMOType = getParentModule()->par("SUMOType").stringValue();
        vehicleClass = getParentModule()->par("vehicleClass").stringValue();

        //this->getParentModule()->subscribe(mobilityStateChangedSignal, this);
        //this->getParentModule()->subscribe(parkingStateChangedSignal, this);

        sendBeaconEvt = new omnetpp::cMessage("beacon evt");
    }
    else if (stage == 1)
    {
        if (dataOnSCH && !mac->isChannelSwitchingActive())
        {
            dataOnSCH = false;
            std::cerr << "App wants to send data on SCH but MAC doesn't use any SCH. Sending all data on CCH \n";
        }

        if(DSRCenabled)
        {
            sendBeaconEvt = new omnetpp::cMessage("BeaconEvt");

            omnetpp::simtime_t firstBeaconOffSet = 0.0;

            // simulate asynchronous channel access
            if (par("avoidBeaconSynchronization").boolValue())
            {
                firstBeaconOffSet = dblrand() * beaconInterval;

                if (mac->isChannelSwitchingActive())
                {
                    if ( beaconInterval.raw() % (mac->getSwitchingInterval().raw()*2))
                        LOG_EVENT << boost::format("The beacon interval '%1%' is smaller than or not a multiple of  one synchronization interval '%2%'. This means that beacons are generated during SCH intervals. \n") % beaconInterval % (2*mac->getSwitchingInterval());

                    firstBeaconOffSet = computeAsynchronousSendingTime(beaconInterval, Veins::type_CCH);
                }
            }

            scheduleAt(omnetpp::simTime() + firstBeaconOffSet, sendBeaconEvt);
        }
    }
}


void BaseWaveApplLayer::finish()
{

}


void BaseWaveApplLayer::receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signalID, omnetpp::cObject* obj, omnetpp::cObject* details)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
        handlePositionUpdate(obj);
    // else if (signalID == parkingStateChangedSignal)
    //    handleParkingUpdate(obj);
}


void BaseWaveApplLayer::handlePositionUpdate(omnetpp::cObject* obj)
{
    ChannelMobilityPtrType const mobility = omnetpp::check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
    curSpeed = mobility->getCurrentSpeed();
}


void BaseWaveApplLayer::handleParkingUpdate(omnetpp::cObject* obj)
{

}


void BaseWaveApplLayer::handleLowerMsg(omnetpp::cMessage* msg)
{
    throw omnetpp::cRuntimeError("Can't handle msg '%s' of kind '%d'", msg->getFullName(), msg->getKind());
}


void BaseWaveApplLayer::handleSelfMsg(omnetpp::cMessage* msg)
{
    if (msg == sendBeaconEvt)
    {
        if(sendBeacons)
            sendBeacon();

        // schedule for next beacon broadcast
        scheduleAt(omnetpp::simTime() + beaconInterval, sendBeaconEvt);
    }
    else
        throw omnetpp::cRuntimeError("Can't handle msg '%s' of kind '%d'", msg->getFullName(), msg->getKind());
}


omnetpp::simtime_t BaseWaveApplLayer::computeAsynchronousSendingTime(omnetpp::simtime_t interval, Veins::t_channel chanType)
{
    // 'interval': the interval length of the periodic message
    // 'chanType': the type of channel, either type_CCH or type_SCH

    /*
     * avoid that periodic messages for one channel type are scheduled in the other channel interval
     * when alternate access is enabled in the MAC

     * start event earlierst in next CCH  (or SCH) interval. For alignment, first find the next CCH interval
     * To find out next CCH, go back to start of current interval and add two or one intervals
     * depending on type of current interval
     */

    omnetpp::simtime_t nextCCH;
    omnetpp::simtime_t switchingInterval = mac->getSwitchingInterval(); // usually 0.050s

    if (mac->isCurrentChannelCCH())
        nextCCH = omnetpp::simTime() - omnetpp::SimTime().setRaw(omnetpp::simTime().raw() % switchingInterval.raw()) + switchingInterval*2;
    else
        nextCCH = omnetpp::simTime() - omnetpp::SimTime().setRaw(omnetpp::simTime().raw() %switchingInterval.raw()) + switchingInterval;

    omnetpp::simtime_t randomOffset = dblrand() * beaconInterval;
    omnetpp::simtime_t firstEvent = nextCCH + randomOffset;

    // check if firstEvent lies within the correct interval and, if not, move to previous interval

    if (firstEvent.raw()  % (2*switchingInterval.raw()) > switchingInterval.raw())
    {
        // firstEvent is within a SCH interval
        if (chanType == Veins::type_CCH)
            firstEvent -= switchingInterval;
    }
    else
    {
        // firstEvent is within a CCH interval, so adjust for SCH messages
        if (chanType == Veins::type_SCH)
            firstEvent += switchingInterval;
    }

    return firstEvent;
}


void BaseWaveApplLayer::sendBeacon()
{

}

}
