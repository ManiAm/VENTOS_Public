/****************************************************************************/
/// @file    BLE_Advertisement.cc
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

#include <05_BLE_Advertisement.h>
#include <BeaconFormat.h>
#include <linux/errno.h>       // include this at the end!

namespace VENTOS {

Define_Module(VENTOS::BLE_Advertisement);

BLE_Advertisement::~BLE_Advertisement()
{

}


void BLE_Advertisement::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        BLE_advertisement = par("BLE_advertisement").boolValue();

        if(!BLE_advertisement)
            return;
    }
}


void BLE_Advertisement::finish()
{
    super::finish();

}


void BLE_Advertisement::handleMessage(cMessage *msg)
{
    super::handleMessage(msg);

}


void BLE_Advertisement::initialize_withTraCI()
{
    super::initialize_withTraCI();

}


void BLE_Advertisement::executeEachTimestep()
{
    super::executeEachTimestep();

    // run this code only once
    static bool wasExecuted = false;
    if (BLE_advertisement && !wasExecuted)
    {
        int dev_id = par("BLE_adv_deviceID").longValue();

        if(dev_id == -1)
        {
            // get the first available BT device
            dev_id = hci_get_route(NULL);
            if (dev_id < 0)
                error("Device is not available");
        }

        int minInterval = par("BLE_minInterval").longValue();
        int maxInterval = par("BLE_maxInterval").longValue();
        uint8_t ADVtype = par("BLE_adv_type").longValue();
        uint8_t channelNumber = par("BLE_channelNumber").longValue();
        int beaconType = par("BLE_beaconType").longValue();

        advertiseBeacon(dev_id, minInterval, maxInterval, ADVtype, channelNumber, beaconType);

        wasExecuted = true;
    }
}


/*
 Commands in linux:
     sudo hciconfig hci0 up
     sudo hciconfig hci0 leadv 3
     sudo hcitool -i hci0 cmd 0x08 0x0008 1e 02 01 1a 1a ff 4c 00 02 15 e2 c5 6d b5 df fb 48 d2 b0 60 d0 f5 a7 10 96 e0 00 00 00 00 c5
 */
void BLE_Advertisement:: advertiseBeacon(int dev_id, int minInterval, int maxInterval, uint8_t ADVtype, uint8_t channelNumber, int beaconType)
{
    // disable LE advertising first
    no_le_adv(dev_id);

    // then, enable LE advertising
    le_adv(dev_id, minInterval, maxInterval, ADVtype, channelNumber);

    // generate BLE advertising PDU
    std::string payload = generateBeacon(beaconType);

    // ogf = 0x08:  Bluetooth Command Group
    // ocf = 0x008: 'LE Set Advertising Data' command
    cmd_cmd(dev_id, 0x08 /*ogf*/, 0x0008 /*ocf*/, payload);
}


void BLE_Advertisement::no_le_adv(int hdev)
{
    if (hdev < 0)
        error("Not a valid device");

    int dd = hci_open_dev(hdev);
    if (dd < 0)
        error("Could not open device");

    std::cout << std::endl << ">>> Disabling LE advertising on hci" << hdev << "... \n" << std::flush;

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


void BLE_Advertisement::le_adv(int hdev, uint16_t minInterval, uint16_t maxInterval, uint8_t ADVtype, uint8_t channel)
{
    if (hdev < 0)
        error("Not a valid device");

    int dd = hci_open_dev(hdev);
    if (dd < 0)
        error("Could not open device");

    std::cout << std::endl << ">>> Enabling LE advertising on hci" << hdev << "... \n";
    std::cout << "    Min interval: " << minInterval;
    std::cout << ", Max interval: " << maxInterval;
    std::cout << ", Advertisement type: " << (int)ADVtype;
    std::cout << ", Channel number: " << (int)channel << std::endl;

    std::cout << std::flush;

    le_set_advertising_parameters_cp adv_params_cp;
    memset(&adv_params_cp, 0, sizeof(adv_params_cp));
    adv_params_cp.min_interval = htobs(minInterval);
    adv_params_cp.max_interval = htobs(maxInterval);
    adv_params_cp.advtype = ADVtype;
    adv_params_cp.chan_map = channel;

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


std::string BLE_Advertisement::generateBeacon(int beaconType)
{
    std::cout << std::endl << ">>> Generating type " << beaconType << " beacon... \n" << std::flush;

    std::vector<std::string> params;  // stores beacon parameters
    std::string payload = "";         // contains beacon payload as a single string

    // iBeacon format: http://stackoverflow.com/questions/18906988/what-is-the-ibeacon-bluetooth-profile
    if(beaconType == iBeacon_Type)
    {
        // iBeacon parameters
        std::string iBeacon_UUID = par("iBeacon_UUID").stringValue();
        std::string iBeacon_major = par("iBeacon_major").stringValue();
        std::string iBeacon_minor = par("iBeacon_minor").stringValue();
        std::string iBeacon_TXpower = par("iBeacon_TXpower").stringValue();

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
        // AltBeacon parameters
        std::string AltBeacon_MFGID = par("AltBeacon_MFGID").stringValue();
        std::string AltBeacon_beaconID = par("AltBeacon_beaconID").stringValue();
        std::string AltBeacon_refRSSI = par("AltBeacon_refRSSI").stringValue();
        std::string AltBeacon_MFGRSVD = par("AltBeacon_MFGRSVD").stringValue();

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
