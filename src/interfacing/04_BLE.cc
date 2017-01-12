/****************************************************************************/
/// @file    BLE.cc
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

#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <linux/errno.h>

#include "interfacing/04_BLE.h"

namespace VENTOS {

#define EIR_FLAGS           0x01  /* flags */
#define EIR_NAME_SHORT      0x08  /* shortened local name */
#define EIR_NAME_COMPLETE   0x09  /* complete local name */
#define EIR_APPEARANCE      0x19  /* Device appearance */

Define_Module(VENTOS::BLE);

BLE::~BLE()
{

}


void BLE::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        BLE_on = par("BLE_on").boolValue();

        if(!BLE_on)
            return;

        boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        cached_LEBT_devices_filePATH = VENTOS_FullPath / "results" / "cached_LEBT_devices.txt";
    }
}


void BLE::finish()
{
    super::finish();

}


void BLE::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);

}


void BLE::initialize_withTraCI()
{
    super::initialize_withTraCI();

}


void BLE::executeEachTimestep()
{
    super::executeEachTimestep();

    // run this code only once
    static bool wasExecuted = false;
    if (BLE_on && !wasExecuted)
    {
        // cached LE BT devices from previous scans
        loadCachedDevices();

        int dev_id = par("BLE_scan_deviceID").longValue();

        if(dev_id == -1)
        {
            // get the first available BT device
            dev_id = hci_get_route(NULL);
            if (dev_id < 0)
                throw omnetpp::cRuntimeError("Device is not available");
        }

        int scan_type = par("BLE_scan_type").longValue();
        uint16_t interval = par("BLE_interval").longValue();
        uint16_t window = par("BLE_window").longValue();
        uint8_t own_type = par("BLE_own_type").longValue();
        uint8_t filter_policy = par("BLE_filter_policy").longValue();
        int scan_time = par("BLE_scan_time").longValue();

        // scan for Low Energy (LE) bluetooth device
        lescan(dev_id, scan_type, interval, window, own_type, filter_policy, scan_time);

        // uint16_t handle = leCreateConnection("CC:4B:DA:B0:F8:28");

        wasExecuted = true;
    }
}


void BLE::loadCachedDevices()
{
    std::ifstream infile(cached_LEBT_devices_filePATH.string().c_str());
    // no such file exists!
    if(infile.fail())
        return;

    int counter = 0;
    std::string line;
    while (std::getline(infile, line))
    {
        std::vector<std::string> tokens = omnetpp::cStringTokenizer(line.c_str(), ",,").asVector();
        if(tokens.size() < 3)
            throw omnetpp::cRuntimeError("file format is not correct!");

        std::string BTaddr = tokens[0] ;
        std::string BTname = tokens[1];
        std::string timeStamp = tokens[2];

        // Remove leading and trailing spaces from string
        boost::trim(BTaddr);
        boost::trim(BTname);
        boost::trim(timeStamp);

        auto it = allLEBTdevices.find(BTaddr);
        if(it == allLEBTdevices.end())
        {
            BTdevEntry *v = new BTdevEntry(BTname, timeStamp);
            allLEBTdevices.insert(std::make_pair(BTaddr, *v));
        }

        counter++;
    }

    std::cout << ">>> " << counter << " cached LE BT devices read from file.";
}


void BLE::saveCachedDevices()
{
    if(allLEBTdevices.empty())
        return;

    FILE *filePtr = fopen (cached_LEBT_devices_filePATH.string().c_str(), "w");
    if(!filePtr)
        throw omnetpp::cRuntimeError("can not open file for writing!");

    for(auto &i : allLEBTdevices)
        fprintf (filePtr, "%s  ,,  %s  ,,  %s \n", i.first.c_str(), i.second.name.c_str(), i.second.timeStamp.c_str());

    fclose(filePtr);
}


void BLE::lescan(int dev_id, uint8_t scan_type, uint16_t interval, uint16_t window, uint8_t own_type, uint8_t filter_policy, int scan_time)
{
    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        throw omnetpp::cRuntimeError("Could not open device");

    std::cout << std::endl << ">>> Scanning Bluetooth LE devices on hci" << dev_id << " for " << scan_time << " seconds... \n";
    std::cout << "    Scan type: " << (int)scan_type;
    std::cout << ", Interval: " << interval;
    std::cout << ", Window: " << window;
    std::cout << ", Own type: " << (int)own_type;
    std::cout << ", Filter policy: " << (int)filter_policy << std::endl;

    std::cout << std::flush;

    int err = hci_le_set_scan_parameters(dd, scan_type, interval, window, own_type, filter_policy, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        throw omnetpp::cRuntimeError("Set scan parameters failed: %s", strerror(errno));
    }

    uint8_t filter_dup = 1;   // filter duplicates
    err = hci_le_set_scan_enable(dd, 0x01 /*enable*/, filter_dup, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        throw omnetpp::cRuntimeError("Enable scan failed");
    }

    struct hci_filter of;
    socklen_t olen = sizeof(of);
    if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0)
    {
        hci_close_dev(dd);
        throw omnetpp::cRuntimeError("Could not get socket options");
    }

    struct hci_filter nf;
    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0)
    {
        hci_close_dev(dd);
        throw omnetpp::cRuntimeError("Could not set socket options");
    }

    auto rsp = print_advertising_devices(dd, scan_time);

    if(rsp.empty())
    {
        std::cout << "No devices found!" << std::endl;
    }
    else
    {
        std::cout << rsp.size() << " devices found: " << std::endl;

        for(auto i : rsp)
        {
            printf("    Name: %s \n", i.second[0].c_str());
            printf("    BD Addr: %s \n", i.first.c_str());
            printf("    OUI: %s \n\n", i.second[1].c_str());

            // save device info
            auto it = allLEBTdevices.find(i.first);
            // new LE BT device
            if(it == allLEBTdevices.end())
            {
                BTdevEntry *v = new BTdevEntry(i.second[0] /*name*/, i.second[2] /*timestamp*/);
                allLEBTdevices.insert( std::make_pair(i.first /*BT address*/, *v) );
            }
            // if LE BT address already exists, then only update name and timestamp
            else
            {
                it->second.name = i.second[0];
                it->second.timeStamp = i.second[2];
            }
        }
    }

    // flush what we have so far
    std::cout.flush();

    saveCachedDevices();

    setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    err = hci_le_set_scan_enable(dd, 0x00 /*disable*/, filter_dup, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        throw omnetpp::cRuntimeError("Disable scan failed");
    }

    hci_close_dev(dd);
}


std::map<std::string /*BT add*/, std::vector<std::string>> BLE::print_advertising_devices(int dd, int timeout)
{
    unsigned char buffer[HCI_MAX_EVENT_SIZE];
    fd_set read_set;

    struct timeval wait;
    wait.tv_sec = timeout;
    wait.tv_usec = 0;

    int ts = time(NULL);  // get current time

    std::map<std::string /*BT add*/, std::vector<std::string>> scannedDevs;
    scannedDevs.clear();

    while(1)
    {
        FD_ZERO(&read_set);
        FD_SET(dd, &read_set);

        int err = select(FD_SETSIZE, &read_set, NULL, NULL, &wait);
        if (err <= 0)
            break;

        int len = read(dd, buffer, sizeof(buffer));

        for (int i=0; i < len; i++)
        {
            unsigned char* ptr = buffer + HCI_EVENT_HDR_SIZE + 1;
            evt_le_meta_event* meta = (evt_le_meta_event*) ptr;

            if (meta->subevent != EVT_LE_ADVERTISING_REPORT)
                continue;

            le_advertising_info* info;
            info = (le_advertising_info*) (meta->data + 1);

            // get name
            std::string name = parse_name(info->data, info->length);

            // get address
            char addr[18];
            ba2str(&info->bdaddr, addr);

            // get current timestamp
            std::string timeDate = currentDateTime();

            // check if this is a new device
            auto it = scannedDevs.find(std::string(addr));
            if(it == scannedDevs.end())
            {
                // get Manufacturer
                const u_int8_t BTaddr[3] = {(&info->bdaddr)->b[5], (&info->bdaddr)->b[4], (&info->bdaddr)->b[3]};
                std::string OUI = EtherPtr->OUITostr(BTaddr);

                // get RSSI value
                //int RSSI = (char)info->data[info->length];

                // get flags
                //int flags = parse_flags(info->data, info->length);

                // get appearance
                //int appearance = parse_appearance(info->data, info->length);

                std::vector<std::string> values = {name, OUI, timeDate};
                scannedDevs[std::string(addr)] = values;
            }
            else
            {
                // only update name and timestamp
                (it->second)[0] = std::string(name);
                (it->second)[2] = timeDate;
            }
        }

        int elapsed = time(NULL) - ts;
        if (timeout != -1 && elapsed >= timeout)
            break;
    }

    return scannedDevs;
}


std::string BLE::parse_name(uint8_t* data, size_t size)
{
    size_t offset = 0;
    std::string unknown = "unknown";

    while (offset < size)
    {
        uint8_t field_len = data[0];
        size_t name_len;

        if (field_len == 0 || offset + field_len > size)
            return unknown;

        switch (data[1])
        {
        case EIR_NAME_SHORT:
        case EIR_NAME_COMPLETE:
            name_len = field_len - 1;

            // remove any trailing \x00s
            if (data[1 + name_len] == '\0')
                name_len--;

            if (name_len > size)
                return unknown;

            return std::string((const char*)(data + 2), name_len);
        }

        offset += field_len + 1;
        data += field_len + 1;
    }

    return unknown;
}


int BLE::parse_flags(uint8_t* data, size_t size)
{
    size_t offset = 0;

    while (offset < size)
    {
        uint8_t field_len = data[0];

        if (field_len == 0 || offset + field_len > size)
        {
            // this probably ought to be an exception
            return 0;
        }

        if (data[1] == EIR_FLAGS)
            return data[2];

        offset += field_len + 1;
        data += field_len + 1;
    }

    return 0;
}


int BLE::parse_appearance(uint8_t* data, size_t size)
{
    size_t offset = 0;

    while (offset < size)
    {
        uint8_t field_len = data[0];

        if (field_len == 0 || offset + field_len > size)
        {
            // this probably ought to be an exception
            return 0;
        }

        if (data[1] == EIR_APPEARANCE)
        {
            uint16_t appearance = ((uint16_t *)(data + 2))[0];
            return appearance;
        }

        offset += field_len + 1;
        data += field_len + 1;
    }

    return 0;
}


// check this: http://stackoverflow.com/questions/16224561/multiple-ble-connections-using-linux-and-bluez-5-0
uint16_t BLE::leCreateConnection(std::string str_bdaddr)
{
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0)
        throw omnetpp::cRuntimeError("Device is not available");

    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        throw omnetpp::cRuntimeError("Could not open device");

    uint16_t interval = htobs(0x0004);
    uint16_t window = htobs(0x0004);
    uint8_t initiator_filter = 0;  // Use peer address
    uint8_t peer_bdaddr_type = LE_PUBLIC_ADDRESS;
    uint8_t own_bdaddr_type = 0x00;
    uint16_t min_interval = htobs(0x000F);
    uint16_t max_interval = htobs(0x000F);
    uint16_t latency = htobs(0x0000);
    uint16_t supervision_timeout = htobs(0x0C80);
    uint16_t min_ce_length = htobs(0x0001);
    uint16_t max_ce_length = htobs(0x0001);
    uint16_t handle;

    bdaddr_t bdaddr;
    memset(&bdaddr, 0, sizeof(bdaddr_t));
    str2ba(str_bdaddr.c_str(), &bdaddr);

    int err = hci_le_create_conn(dd, interval, window, initiator_filter,
            peer_bdaddr_type, bdaddr, own_bdaddr_type, min_interval,
            max_interval, latency, supervision_timeout,
            min_ce_length, max_ce_length, &handle, 10000 /*timeout milliseconds*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        throw omnetpp::cRuntimeError("Could not create connection");
    }

    printf("Connection handle %d \n", handle);

    hci_close_dev(dd);

    return handle;
}

}
