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

        // iBeacon parameters
        iBeacon_UUID = par("iBeacon_UUID").stringValue();
        iBeacon_major = par("iBeacon_major").stringValue();
        iBeacon_minor = par("iBeacon_minor").stringValue();
        iBeacon_TXpower = par("iBeacon_TXpower").stringValue();

        // AltBeacon parameters
        AltBeacon_MFGID = par("AltBeacon_MFGID").stringValue();
        AltBeacon_beaconID = par("AltBeacon_beaconID").stringValue();
        AltBeacon_refRSSI = par("AltBeacon_refRSSI").stringValue();
        AltBeacon_MFGRSVD = par("AltBeacon_MFGRSVD").stringValue();

        // display local devices
        getLocalDevs();

        std::cout.flush();
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

    // generate BLE advertising PDU
    std::string payload = generateBeacon();

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
    adv_params_cp.advtype = type;
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


std::string AdvertiseBeacon::generateBeacon()
{
    std::vector<std::string> params;  // stores beacon parameters
    std::string payload = "";         // contains beacon payload as a single string

    // iBeacon format: http://stackoverflow.com/questions/18906988/what-is-the-ibeacon-bluetooth-profile
    if(beaconType == iBeacon_Type)
    {
        /* ADV flags:
           02 # Number of bytes that follow in first AD structure
           01 # Flags AD type
           1A # Flags value 0x1A = 000011010
               bit 0 (OFF) LE Limited Discoverable Mode
               bit 1 (ON) LE General Discoverable Mode
               bit 2 (OFF) BR/EDR Not Supported
               bit 3 (ON) Simultaneous LE and BR/EDR to Same Device Capable (controller)
               bit 4 (ON) Simultaneous LE and BR/EDR to Same Device Capable (Host)
         */
        params.push_back("02 01 1a");

        /* ADV header:
           1A # Number of bytes that follow in second (and last) AD structure
           FF # Manufacturer specific data AD type
         */
        params.push_back("1a ff");

        // Company identifier code (https://www.bluetooth.com/specifications/assigned-numbers/company-Identifiers)
        // 0x004C == Apple
        // 0x0059 == Nordic Semi.
        // 0x0078 == Nike
        // 0x015D == Estimote
        // 0x0171 == Amazon Fulfillment Service
        params.push_back("4c 00");

        // iBeacon advertisement indicator
        params.push_back("02");  // Byte 0 (iBeacon type)
        params.push_back("15");  // Byte 1 (iBeacon length)

        params.push_back(iBeacon_UUID);
        params.push_back(iBeacon_major);
        params.push_back(iBeacon_minor);

        // The 2's complement of the calibrated Tx Power
        // More on transmission power here: https://github.com/AltBeacon/altbeacon-transmitter-android
        params.push_back(iBeacon_TXpower);

        // generate an iBeacon message
        iBeacon *msg = new iBeacon(params);

        // pre-pending number of significant data octets (max of 31)
        payload = msg->size + " " + msg->payload;
    }
    else if(beaconType == AltBeacon_Type)
    {
        // AD length
        params.push_back("1b");

        // AD type
        params.push_back("ff");

        params.push_back(AltBeacon_MFGID);
        params.push_back("be ac");  // beacon code should be 0xBEAC
        params.push_back(AltBeacon_beaconID);
        params.push_back("AltBeacon_refRSSI");
        params.push_back(AltBeacon_MFGRSVD);

        // generate an AltBeacon message
        AltBeacon *msg = new AltBeacon(params);

        // pre-pending number of significant data octets
        payload = msg->size + " " + msg->payload;
    }
    else
        error("beaconType %d is not valid!", beaconType);

    return payload;
}


}
