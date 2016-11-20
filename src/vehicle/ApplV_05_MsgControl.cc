/****************************************************************************/
/// @file    ApplV_05_MsgControl.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Nov 2016
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

#include "ApplV_05_MsgControl.h"

#include "PhyToMacControlInfo.h"
#include "DeciderResult80211.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVMsgControl);

ApplVMsgControl::~ApplVMsgControl()
{

}


void ApplVMsgControl::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        printCtrlData = par("printCtrlData").boolValue();
    }
}


void ApplVMsgControl::finish()
{
    super::finish();
}


// handle my own SelfMsg
void ApplVMsgControl::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplVMsgControl::onBeaconVehicle(BeaconVehicle* wsm)
{
    super::onBeaconVehicle(wsm);

    Veins::WaveShortMessage* msg = dynamic_cast<Veins::WaveShortMessage*>(wsm);
    ASSERT(msg);
    getControlInfo(msg);
}


void ApplVMsgControl::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    // super::onBeaconPedestrian(wsm);

    Veins::WaveShortMessage* msg = dynamic_cast<Veins::WaveShortMessage*>(wsm);
    ASSERT(msg);
    getControlInfo(msg);
}


void ApplVMsgControl::onBeaconRSU(BeaconRSU* wsm)
{
    super::onBeaconRSU(wsm);

    Veins::WaveShortMessage* msg = dynamic_cast<Veins::WaveShortMessage*>(wsm);
    ASSERT(msg);
    getControlInfo(msg);
}


void ApplVMsgControl::onPlatoonMsg(PlatoonMsg* wsm)
{
    super::onPlatoonMsg(wsm);

    Veins::WaveShortMessage* msg = dynamic_cast<Veins::WaveShortMessage*>(wsm);
    ASSERT(msg);
    getControlInfo(msg);
}


void ApplVMsgControl::getControlInfo(Veins::WaveShortMessage *msg)
{
    if(printCtrlData)
    {
        // get the control info attached to this msg
        PhyToMacControlInfo *control = (PhyToMacControlInfo *) msg->getControlInfo();
        ASSERT(control);
        DeciderResult80211 *decider = dynamic_cast<DeciderResult80211 *> (control->getDeciderResult());
        ASSERT(decider);

        // print
        LOG_EVENT_C(SUMOID, "default") << boost::format("%1%: Received msg '%2%' with ") % omnetpp::simTime().dbl() % msg->getFullName();
        LOG_EVENT_C(SUMOID, "default") << boost::format("Bitrate: '%.2f', Received power '%4.3f' dBm, SNR: '%.3f', collision: '%d' \n") % decider->getBitrate() % decider->getRecvPower_dBm() % decider->getSnr() % decider->isCollision();
        LOG_FLUSH_C(SUMOID, "default");

        /*
         * bitrate: the bit-rate of the transmission of the packet
         * snr: the signal to noise ratio of the transmission
         * recvPower_dBm: the received power in dBm
         *                Please note that this is NOT the RSSI. The RSSI is an indicator
         *                of the quality of the signal which is not standardized, and
         *                different vendors can define different indicators. This value
         *                indicates the power that the frame had when received by the
         *                NIC card, WITHOUT noise floor and WITHOUT interference
         * collision: if the uncorrect decoding was due to low power or collision
         */
    }
}


// is called, every time the position of vehicle changes
void ApplVMsgControl::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);

}

}
