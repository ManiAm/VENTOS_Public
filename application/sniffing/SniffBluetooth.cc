/****************************************************************************/
/// @file    SniffBluetooth.cc
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

#include <SniffBluetooth.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// service discovery
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

namespace VENTOS {

Define_Module(VENTOS::SniffBluetooth);

SniffBluetooth::~SniffBluetooth()
{

}


void SniffBluetooth::initialize(int stage)
{
    if(stage == 0)
    {
        on = par("on").boolValue();

        if(!on)
            return;

        interface = par("interface").stdstringValue();

        // register signals
        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);
    }
}


void SniffBluetooth::finish()
{


}


void SniffBluetooth::handleMessage(cMessage *msg)
{


}


void SniffBluetooth::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        SniffBluetooth::executeEachTimestep();
    }
    else if(signalID == Signal_executeFirstTS)
    {
        SniffBluetooth::executeFirstTimeStep();
    }
}


void SniffBluetooth::executeFirstTimeStep()
{

}


void SniffBluetooth::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (on && !wasExecuted)
    {
        // looking for nearby devices
        //scanNearbyDevices();

        // request for service
        serviceDiscovery("30:75:12:6D:B2:3A");

        wasExecuted = true;
    }
}


void SniffBluetooth::scanNearbyDevices()
{
    int dev_id = hci_get_route(NULL);  // get the first available Bluetooth adapter
    int sock = hci_open_dev(dev_id);
    if (dev_id < 0 || sock < 0)
        error("opening socket");

    int len  = 8;       // inquiry lasts for at most 1.28 * len seconds (= 10.24 seconds)
    int max_rsp = 255;  // at most max_rsp devices will be returned
    int flags = IREQ_CACHE_FLUSH; // the cache of previously detected devices is flushed before performing the current inquiry
    inquiry_info *ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    std::cout << "Starting Bluetooth device discovery... " << std::flush;
    // num_rsp contains the number of discovered devices
    // ii contains the discovered devices info
    int num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);

    if(num_rsp < 0)
    {
        error("hci_inquiry");
    }
    else if(num_rsp == 0)
    {
        std::cout << "Done!" << std::endl;
        std::cout << "No devices found!" << std::endl;
    }
    else if(num_rsp > 0)
    {
        std::cout << "Done!" << std::endl;
        std::cout << "List of found devices: " << std::endl;

        char addr[19] = { 0 };
        char name[248] = { 0 };
        for (int i = 0; i < num_rsp; i++)
        {
            // get user-friendly name associated with this device
            memset(name, 0, sizeof(name));
            int ret = hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0 /*timeout milliseconds*/);
            // on failure
            if (ret < 0) strcpy(name, "[unknown]");
            printf("  name: %s \n", name);

            // get device address
            ba2str(&(ii+i)->bdaddr, addr);
            printf("      address: %s \n", addr);

            // get device class
            std::string dev_class = classInfo((ii+i)->dev_class);
            printf("      device_class: %s \n", dev_class.c_str());

            // get more detailed info
            printf("      pscan_rep_mode: %02x \n", (ii+i)->pscan_rep_mode);
            printf("      pscan_mode: %02x \n", (ii+i)->pscan_mode);
            printf("      clock_offset: %04x \n\n", (ii+i)->clock_offset);
        }
    }

    free(ii);
    close(sock);
}


// Decode device class
// from https://www.bluetooth.com/specifications/assigned-numbers/baseband
std::string SniffBluetooth::classInfo(uint8_t dev_class[3])
{
    std::ostringstream os;
    os.str("");

    int flags = dev_class[2];
    int majorNum = dev_class[1];
    int minorNum = dev_class[0] >> 2;

    os << "[ ";
    if (flags & 0x1)  os << "Positioning ";
    if (flags & 0x2)  os << "Networking ";
    if (flags & 0x4)  os << "Rendering ";
    if (flags & 0x8)  os << "Capturing ";
    if (flags & 0x10) os << "Object_Transfer ";
    if (flags & 0x20) os << "Audio ";
    if (flags & 0x40) os << "Telephony ";
    if (flags & 0x80) os << "Information ";
    os << "] ";

    auto it = majors.find(majorNum);
    if(it == majors.end())
    {
        os << "Not a valid major number";
        return os.str();
    }

    // get major name
    std::string majorName = it->second;
    os << majorName << " ";

    if(majorName == "Computer")
    {
        auto it2 = computers.find(minorNum);

        if(it2 == computers.end())
            os << "Not a valid minor number";
        else
            os << it2->second;
    }
    else if(majorName == "Phone")
    {
        auto it2 = phones.find(minorNum);

        if(it2 == phones.end())
            os << "Not a valid minor number";
        else
            os << it2->second;
    }
    else if(majorName == "Net Access Point")
    {
        os << "Usage " << std::to_string(minorNum / 56);
    }
    else if(majorName == "Audio/Video")
    {
        auto it2 = av.find(minorNum);

        if(it2 == av.end())
            os << "Not a valid minor number";
        else
            os << it2->second;
    }
    else if(majorName == "Peripheral")
    {
        auto it2 = peripheral.find(minorNum & 0xF);

        if(it2 == peripheral.end())
            os << "Not a valid minor number";
        else
            os << it2->second;

        if (minorNum & 0x10) os << " with keyboard";
        if (minorNum & 0x20) os << " with pointing device";
        if (minorNum & 0x30) os << " with keyboard and pointing device";
    }
    else if(majorName == "Imaging")
    {
        if (minorNum & 0x2) os <<  " with display";
        if (minorNum & 0x4) os <<  " with camera";
        if (minorNum & 0x8) os <<  " with scanner";
        if (minorNum & 0x10) os << " with printer";
    }
    else if(majorName == "Wearable")
    {
        auto it2 = wearable.find(minorNum);

        if(it2 == wearable.end())
            os << "Not a valid minor number";
        else
            os << it2->second;
    }
    else if(majorName == "Toy")
    {
        auto it2 = toys.find(minorNum);

        if(it2 == toys.end())
            os << "Not a valid minor number";
        else
            os << it2->second;
    }

    return os.str();
}


void SniffBluetooth::serviceDiscovery(std::string bdaddr, uint16_t UUID)
{
    bdaddr_t src = {0, 0, 0, 0, 0, 0};  // broadcast address
    bdaddr_t dst;
    str2ba(bdaddr.c_str(), &dst);

    // connect to the SDP server running on the remote machine
    sdp_session_t *session = sdp_connect(&src, &dst, SDP_RETRY_IF_BUSY);

    if(!session)
    {
        std::cout << "Failed to connect to SDP server!" << std::endl;
        return;
    }

    // if no UUID is specified then browse
    if(UUID == 0)
        UUID = PUBLIC_BROWSE_GROUP;

    // build linked lists
    uuid_t root_uuid;
    sdp_uuid16_create(&root_uuid, UUID);
    uint32_t range = 0x0000ffff;
    sdp_list_t *attrid = sdp_list_append(0, &range);
    sdp_list_t *search = sdp_list_append(0, &root_uuid);

    // get a linked list of services
    sdp_list_t *seq = NULL;
    int err = sdp_service_search_attr_req(session, search, SDP_ATTR_REQ_RANGE, attrid, &seq);
    if(err < 0)
    {
        std::cout << "Timeout in SDP service search!" << std::endl;
        sdp_close(session);
        return;
    }

    sdp_list_free(attrid, 0);
    sdp_list_free(search, 0);

    // loop through the list of services
    for(; seq; seq = seq->next)
    {
        sdp_record_t *rec = (sdp_record_t *) seq->data;

        // print the service name
        sdp_record_print(rec);

        // get a list of the protocol sequences
        sdp_list_t *proto_list = NULL;
        sdp_get_access_protos(rec, &proto_list);

        // go through each protocol sequence
        for(sdp_list_t *p = proto_list; p; p = p->next )
        {
            // go through each protocol list of the protocol sequence
            for(sdp_list_t *pds = (sdp_list_t*) p->data; pds; pds = pds->next)
            {
                int proto = -1;
                std::string protocolName = "unknown";
                std::map<unsigned int, std::string>::const_iterator it;

                // check the protocol attributes
                for(sdp_data_t *d = (sdp_data_t*) pds->data; d; d = d->next)
                {
                    switch(d->dtd)
                    {
                    case SDP_UUID16:
                    case SDP_UUID32:
                    case SDP_UUID128:
                        proto = sdp_uuid_to_proto(&d->val.uuid);
                        it = ProtocolUUIDs.find(proto);
                        if(it != ProtocolUUIDs.end())
                            protocolName = it->second;
                        printf("    protocol: 0x%04x (%s) \n", proto, protocolName.c_str());
                        break;
                    case SDP_UINT8:
                        if(proto == RFCOMM_UUID) printf("    RFCOMM channel: %d \n", d->val.int8);
                        break;
                    }
                }
            }
        }
    }

    free(seq);
    sdp_close(session);
}


void SniffBluetooth::startSniffing()
{

}


void SniffBluetooth::got_packet(const struct pcap_pkthdr *header, const u_char *packet)
{

}

}
