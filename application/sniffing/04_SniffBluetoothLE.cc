/****************************************************************************/
/// @file    SniffBluetoothLE.cc
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

#include <04_SniffBluetoothLE.h>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <linux/errno.h>

namespace VENTOS {

#define EIR_FLAGS           0x01  /* flags */
#define EIR_NAME_SHORT      0x08  /* shortened local name */
#define EIR_NAME_COMPLETE   0x09  /* complete local name */
#define EIR_APPEARANCE      0x19  /* Device appearance */

Define_Module(VENTOS::SniffBluetoothLE);

SniffBluetoothLE::~SniffBluetoothLE()
{

}


void SniffBluetoothLE::initialize(int stage)
{
    SniffBluetooth::initialize(stage);

    if(stage == 0)
    {
        LEBTon = par("LEBTon").boolValue();

        if(!LEBTon)
            return;

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        cached_LEBT_devices_filePATH = VENTOS_FullPath / "application/sniffing/cached_LEBT_devices";
    }
}


void SniffBluetoothLE::finish()
{
    SniffBluetooth::finish();

}


void SniffBluetoothLE::handleMessage(cMessage *msg)
{
    SniffBluetooth::handleMessage(msg);

}


void SniffBluetoothLE::executeFirstTimeStep()
{
    SniffBluetooth::executeFirstTimeStep();

}


void SniffBluetoothLE::executeEachTimestep()
{
    SniffBluetooth::executeEachTimestep();

    // run this code only once
    static bool wasExecuted = false;
    if (LEBTon && !wasExecuted)
    {
        // display local devices
        getLocalDevs();

        // cached LE BT devices from previous scans
        loadCachedDevices();

        // scan for Low Energy (LE) bluetooth device
        //lescan();

        // uint16_t handle = leCreateConnection("CC:4B:DA:B0:F8:28");

        wasExecuted = true;
    }
}


void SniffBluetoothLE::loadCachedDevices()
{
    std::ifstream infile(cached_LEBT_devices_filePATH.string().c_str());
    // no such file exists!
    if(infile.fail())
        return;

    int counter = 0;
    std::string line;
    while (std::getline(infile, line))
    {
        std::vector<std::string> tokens = cStringTokenizer(line.c_str(), ",,").asVector();
        if(tokens.size() < 3)
            error("file format is not correct!");

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
            LEBTdevEntry *v = new LEBTdevEntry(BTname, timeStamp);
            allLEBTdevices.insert(std::make_pair(BTaddr, *v));
        }

        counter++;
    }

    std::cout << ">>> " << counter << " cached LE BT devices read from file.";
}


void SniffBluetoothLE::saveCachedDevices()
{
    if(allLEBTdevices.empty())
        return;

    FILE *filePtr = fopen (cached_LEBT_devices_filePATH.string().c_str(), "w");
    if(!filePtr)
        error("can not open file for writing!");

    for(auto &i : allLEBTdevices)
        fprintf (filePtr, "%s  ,,  %s  ,,  %s \n", i.first.c_str(), i.second.name.c_str(), i.second.timeStamp.c_str());

    fclose(filePtr);
}


void SniffBluetoothLE::lescan()
{
    int dev_id = hci_get_route(NULL);  // get the first available Bluetooth adapter
    if(dev_id < 0)
        error("Device is not available");

    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Could not open device");

    std::cout << std::endl << ">>> Scan for Bluetooth LE devices... " << std::flush;

    uint8_t scan_type = 0x01;  // 0x00: passive, 0x01: active  More info: http://stackoverflow.com/questions/24994776/android-ble-passive-scan
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);
    uint8_t own_type = 0x00;  // 0x01: enable privacy
    uint8_t filter_policy = 0x00;  // 0x00: accept all, 0x01: white_list
    int err = hci_le_set_scan_parameters(dd, scan_type, interval, window, own_type, filter_policy, 1000);
    if (err < 0)
    {
        hci_close_dev(dd);
        error("Set scan parameters failed");
    }

    uint8_t filter_dup = 1;
    err = hci_le_set_scan_enable(dd, 0x01 /*enable*/, filter_dup, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        error("Enable scan failed");
    }

    struct hci_filter of;
    socklen_t olen = sizeof(of);
    if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0)
    {
        hci_close_dev(dd);
        error("Could not get socket options");
    }

    struct hci_filter nf;
    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0)
    {
        hci_close_dev(dd);
        error("Could not set socket options");
    }

    print_advertising_devices(dd, 10 /*timeout in seconds*/);

    std::cout << "Done!" << std::endl;

    if(allLEBTdevices.empty())
    {
        std::cout << "No devices found!" << std::endl;
    }
    else
    {
        std::cout << allLEBTdevices.size() << " devices found: " << std::endl;
        for(auto i : allLEBTdevices)
        {
            printf("    addr: %s \n", i.first.c_str());
            printf("    name: %s \n", i.second.name.c_str());
        }
    }

    // flush what we have
    std::cout.flush();

    saveCachedDevices();

    setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    err = hci_le_set_scan_enable(dd, 0x00 /*disable*/, filter_dup, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        error("Disable scan failed");
    }

    hci_close_dev(dd);
}


void SniffBluetoothLE::print_advertising_devices(int dd, int timeout)
{
    unsigned char buffer[HCI_MAX_EVENT_SIZE];
    fd_set read_set;

    struct timeval wait;
    wait.tv_sec = timeout;
    wait.tv_usec = 0;

    int ts = time(NULL);  // get current time

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

            // get address
            char addr[18];
            ba2str(&info->bdaddr, addr);

            // printf("RSSI %d \n", (char)info->data[info->length]);

            // get name
            std::string name = parse_name(info->data, info->length);

            // get flags
            //int flags = parse_flags(info->data, info->length);

            // get appearance
            //int appearance = parse_appearance(info->data, info->length);

            // save device info
            std::string timeDate = currentDateTime();
            auto it = allLEBTdevices.find(std::string(addr));
            // new LE BT device
            if(it == allLEBTdevices.end())
            {
                LEBTdevEntry *v = new LEBTdevEntry(std::string(name), timeDate);
                allLEBTdevices.insert( std::make_pair(std::string(addr), *v) );
            }
            // if LE BT address already exists, then only update name and timestamp
            else
            {
                it->second.name = std::string(name);
                it->second.timeStamp = timeDate;
            }
        }

        int elapsed = time(NULL) - ts;
        if (elapsed >= timeout)
            break;

        wait.tv_sec = timeout - elapsed;
    }
}


int SniffBluetoothLE::parse_flags(uint8_t* data, size_t size)
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


int SniffBluetoothLE::parse_appearance(uint8_t* data, size_t size)
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


std::string SniffBluetoothLE::parse_name(uint8_t* data, size_t size)
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


uint16_t SniffBluetoothLE::leCreateConnection(std::string str_bdaddr)
{
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0)
        error("Device is not available");

    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Could not open device");

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
        error("Could not create connection");
    }

    printf("Connection handle %d \n", handle);

    hci_close_dev(dd);

    return handle;
}


void SniffBluetoothLE::cmd_cmd(int dev_id, uint8_t ogf, uint16_t ocf, std::string payload)
{
    if (dev_id < 0)
        error("Not a valid device");

    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Device open failed");

    std::cout << std::endl << ">>> Sending command... " << std::endl << std::flush;

    std::vector<std::string> tokens = cStringTokenizer(payload.c_str()).asVector();
    int len = tokens.size();

    unsigned char buf[HCI_MAX_EVENT_SIZE];
    unsigned char *ptr = buf;
    for (auto &i : tokens)
        *ptr++ = (uint8_t) strtol(i.c_str(), NULL, 16);

    /* Setup filter */
    struct hci_filter flt;
    hci_filter_clear(&flt);
    hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
    hci_filter_all_events(&flt);
    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
    {
        hci_close_dev(dd);
        error("HCI filter setup failed");
    }

    printf("    < HCI Command: ogf 0x%02x, ocf 0x%04x, plen %d \n", ogf, ocf, len);
    hex_dump("    ", 200, buf, len);
    fflush(stdout);

    if (hci_send_cmd(dd, ogf, ocf, len, buf) < 0)
    {
        hci_close_dev(dd);
        error("Send failed");
    }

    len = read(dd, buf, sizeof(buf));
    if (len < 0)
    {
        hci_close_dev(dd);
        error("Read failed");
    }

    hci_event_hdr *hdr = (hci_event_hdr *)(buf + 1);
    ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
    len -= (1 + HCI_EVENT_HDR_SIZE);

    printf("    > HCI Event: 0x%02x plen %d \n", hdr->evt, hdr->plen);
    hex_dump("    ", 200, ptr, len);
    fflush(stdout);

    hci_close_dev(dd);
}


void SniffBluetoothLE::hex_dump(std::string pref, int width, unsigned char *buf, int len)
{
    register int i,n;

    for (i = 0, n = 1; i < len; i++, n++)
    {
        if (n == 1)
            printf("%s", pref.c_str());

        printf("%2.2X ", buf[i]);

        if (n == width)
        {
            printf("\n");
            n = 0;
        }
    }

    if (i && n != 1)
        printf("\n");
}

}
