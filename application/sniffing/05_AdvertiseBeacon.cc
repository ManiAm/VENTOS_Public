/****************************************************************************/
/// @file    SniffBluetoothLEAdv.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Feb 2016
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

#include <05_AdvertiseBeacon.h>
#include <05_BeaconFormat.h>
#include <linux/errno.h>       // include this at the end!

namespace VENTOS {

/* Advertising report event types */
#define ADV_IND         0x00    // Connectable undirected advertising
#define ADV_DIRECT_IND  0x01    // Connectable high duty cycle directed advertising
#define ADV_SCAN_IND    0x02    // Scannable undirected advertising
#define ADV_NONCONN_IND 0x03    // Non connectable undirected advertising
#define ADV_SCAN_RSP    0x04

Define_Module(VENTOS::AdvertiseBeacon);

AdvertiseBeacon::~AdvertiseBeacon()
{

}


void AdvertiseBeacon::initialize(int stage)
{
    SniffBluetoothLE::initialize(stage);

    if(stage == 0)
    {
        advertisement = par("advertisement").boolValue();

        if(!advertisement)
            return;

        beaconType = par("beaconType").longValue();

        // iBeacon parameter
        UUID = par("UUID").stringValue();
        major = par("major").stringValue();
        minor = par("minor").stringValue();
        TXpower = par("TXpower").stringValue();
    }
}


void AdvertiseBeacon::finish()
{
    SniffBluetoothLE::finish();

}


void AdvertiseBeacon::handleMessage(cMessage *msg)
{
    SniffBluetoothLE::handleMessage(msg);

}


void AdvertiseBeacon::executeFirstTimeStep()
{
    SniffBluetoothLE::executeFirstTimeStep();

}


void AdvertiseBeacon::executeEachTimestep()
{
    SniffBluetoothLE::executeEachTimestep();

    // run this code only once
    static bool wasExecuted = false;
    if (advertisement && !wasExecuted)
    {
        // display local devices
        getLocalDevs();

        advertiseBeacon();

        wasExecuted = true;
    }
}


void AdvertiseBeacon:: advertiseBeacon()
{
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0)
        error("Device is not available");

    // disable LE advertising first
    no_le_adv(dev_id);

    // then, enable LE advertising
    le_adv(dev_id, ADV_NONCONN_IND);

    std::vector<std::string> params;  // stores beacon parameters
    std::string payload = "";         // contains beacon payload as a single string

    if(beaconType == iBeaconType)
    {
        params.push_back("02 01 1a");  // adv flags
        params.push_back("1a ff");     // adv header
        params.push_back("4c 00");     // company id
        params.push_back("02");        // iBeacon type
        params.push_back("15");        // iBeacon length
        params.push_back(UUID);
        params.push_back(major);
        params.push_back(minor);
        params.push_back(TXpower);

        // generate an iBeacon message
        iBeacon *msg = new iBeacon(params);

        // pre-pending number of significant data octets (max of 31)
        payload = msg->size + " " + msg->payload;
    }
    // todo
    else if(beaconType == AltBeaconType)
    {

    }
    else
        error("beaconType %d is not valid!", beaconType);

    // ogf = 0x08:  Bluetooth Command Group
    // ocf = 0x008: 'LE Set Advertising Data' command
    cmd_cmd(dev_id, 0x08 /*ogf*/, 0x0008 /*ocf*/, payload);
}


void AdvertiseBeacon::no_le_adv(int hdev)
{
    if (hdev < 0)
        error("Not a valid device");

    int dd = hci_open_dev(hdev);
    if (dd < 0)
        error("Could not open device");

    std::cout << std::endl << ">>> Disabling LE advertising... " << std::endl << std::flush;

    le_set_advertise_enable_cp advertise_cp;
    memset(&advertise_cp, 0, sizeof(advertise_cp));

    uint8_t status;
    struct hci_request rq;
    memset(&rq, 0, sizeof(rq));

    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
    rq.cparam = &advertise_cp;
    rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    int ret = hci_send_req(dd, &rq, 1000);
    if (ret < 0)
    {
        hci_close_dev(dd);
        error("Can't set advertise mode on hci%d: %s (%d)\n", hdev, strerror(errno), errno);
    }

    hci_close_dev(dd);

    if (status)
        printf("LE set advertise enable on hci%d returned status %d\n", hdev, status);
}


void AdvertiseBeacon::le_adv(int hdev, uint8_t type)
{
    if (hdev < 0)
        error("Not a valid device");

    int dd = hci_open_dev(hdev);
    if (dd < 0)
        error("Could not open device");

    std::cout << std::endl << ">>> Enabling LE advertising... " << std::endl << std::flush;

    le_set_advertising_parameters_cp adv_params_cp;
    memset(&adv_params_cp, 0, sizeof(adv_params_cp));
    adv_params_cp.min_interval = htobs(0x0800);
    adv_params_cp.max_interval = htobs(0x0800);
    if (type != -1) adv_params_cp.advtype = type;
    adv_params_cp.chan_map = 7;  // all channels are enabled, check page 966 of BT spec 4.1

    uint8_t status;
    struct hci_request rq;
    memset(&rq, 0, sizeof(rq));

    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
    rq.cparam = &adv_params_cp;
    rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    int ret = hci_send_req(dd, &rq, 1000);
    if (ret < 0)
    {
        hci_close_dev(dd);
        error("Can't set advertise mode on hci%d: %s (%d)\n", hdev, strerror(errno), errno);
    }

    le_set_advertise_enable_cp advertise_cp;
    memset(&advertise_cp, 0, sizeof(advertise_cp));
    advertise_cp.enable = 0x01;

    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
    rq.cparam = &advertise_cp;
    rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    ret = hci_send_req(dd, &rq, 1000);
    if (ret < 0)
    {
        hci_close_dev(dd);
        error("Can't set advertise mode on hci%d: %s (%d)\n", hdev, strerror(errno), errno);
    }

    hci_close_dev(dd);

    if (status)
        printf("LE set advertise enable on hci%d returned status %d \n", hdev, status);
}

}
