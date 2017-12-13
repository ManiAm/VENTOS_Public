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

#include "nodes/vehicle/yourCode.h"
#include "baseAppl/ApplToPhyControlInfo.h"
#include "MIXIM_veins/nic/phy/PhyToMacControlInfo.h"
#include "MIXIM_veins/nic/phy/decider/DeciderResult80211.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVYourCode);

ApplVYourCode::~ApplVYourCode()
{

}


void ApplVYourCode::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        printCtrlData = par("printCtrlData").boolValue();
        sendingData = par("sendingData").boolValue();
    }
}


void ApplVYourCode::finish()
{
    super::finish();
}


void ApplVYourCode::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplVYourCode::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    super::onBeaconVehicle(wsm);

    if(printCtrlData)
        printControlInfo(dynamic_cast<Veins::WaveShortMessage*>(wsm));
}


void ApplVYourCode::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    super::onBeaconRSU(wsm);

    if(printCtrlData)
        printControlInfo(dynamic_cast<Veins::WaveShortMessage*>(wsm));

    if(sendingData)
    {
        LOG_INFO << boost::format("Vehicle '%s' received a beacon from RSU '%s'. \n") % SUMOID % wsm->getSender();

        dataMsg* data = generateData();

        LOG_INFO << boost::format("Vehicle '%s' is broadcasting a data frame. \n\n") % SUMOID;

        // flush what we have in the output buffer so far!
        LOG_INFO << std::flush;

        // broadcast the data wirelessly using IEEE 802.11p
        send(data, lowerLayerOut);
    }
}


void ApplVYourCode::onDataMsg(dataMsg *wsm)
{
    // do not pass it down!

    if(printCtrlData)
        printControlInfo(dynamic_cast<Veins::WaveShortMessage*>(wsm));
}


dataMsg* ApplVYourCode::generateData()
{
    dataMsg* data = new dataMsg("genericData", TYPE_GENERIC_DATA);

    // header
    data->setWsmVersion(1);
    data->setSecurityType(1);
    data->setChannelNumber(Veins::Channels::CCH);
    data->setDataRate(1);
    data->setPriority(beaconPriority);
    data->setPsid(0);

    data->setSender(SUMOID.c_str());
    data->setTarget(3);

    // add header length
    data->addBitLength(headerLength);

    // add payload length
    data->addBitLength(beaconLengthBits);

    return data;
}


void ApplVYourCode::printControlInfo(Veins::WaveShortMessage *wsm)
{
    if(!printCtrlData)
        return;

    // make sure wsm is not null
    ASSERT(wsm);

    // get the control info attached to this wsm
    PhyToMacControlInfo *control = (PhyToMacControlInfo *) wsm->getControlInfo();
    ASSERT(control);
    DeciderResult80211 *decider = dynamic_cast<DeciderResult80211 *> (control->getDeciderResult());
    ASSERT(decider);

    GLOG(SUMOID, "default") << boost::format("%.6f: Received wsm '%s' with ") %
            omnetpp::simTime().dbl() %
            wsm->getFullName();

    GLOG(SUMOID, "default") << boost::format("Bitrate: '%.2f', Received power '%4.3f' dBm (%.14f mW), SNR: '%.3f dBm' (%.3f) \n") %
            decider->getBitrate() %
            decider->getRecvPower_dBm() %
            pow(10., (decider->getRecvPower_dBm() / 10.)) %
            (10 * log(decider->getSnr())) %
            decider->getSnr();

    GLOG_FLUSH(SUMOID, "default");
}

}
