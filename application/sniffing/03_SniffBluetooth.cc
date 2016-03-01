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

#include <03_SniffBluetooth.h>
#include <02_SniffUSB.h>

#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <sys/ioctl.h>
#include <linux/errno.h>

namespace VENTOS {

Define_Module(VENTOS::SniffBluetooth);

SniffBluetooth::~SniffBluetooth()
{

}


void SniffBluetooth::initialize(int stage)
{
    if(stage == 0)
    {
        BTon = par("BTon").boolValue();

        if(!BTon)
            return;

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        cached_BT_devices_filePATH = VENTOS_FullPath / "application/sniffing/cached_BT_devices";

        // get BT chip on this machine
        getBTchip();

        // display local devices
        getLocalDevs();

        std::cout.flush();
    }
}


void SniffBluetooth::finish()
{


}


void SniffBluetooth::handleMessage(cMessage *msg)
{


}


void SniffBluetooth::executeFirstTimeStep()
{

}


void SniffBluetooth::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (BTon && !wasExecuted)
    {
        // cached BT devices from previous scans
        loadCachedDevices();

        // looking for nearby BT devices
        scan();

        // request for service
        serviceDiscovery("30:75:12:6D:B2:3A");
        serviceDiscovery("4C:A5:6D:58:7C:14");

        wasExecuted = true;
    }
}


void SniffBluetooth::getBTchip()
{
    // get a pointer to the USB module
    cModule *module = simulation.getSystemModule()->getSubmodule("sniffUSB");
    SniffUSB *USBptr = static_cast<SniffUSB *>(module);
    ASSERT(USBptr);

    std::cout << std::endl << ">>> Bluetooth USB devices found on this machine: \n";
    USBptr->getUSBdevices(false /*do not show Configuration Descriptor*/, "bluetooth");
}


void SniffBluetooth::getLocalDevs()
{
    std::cout << std::endl << ">>> Local Bluetooth devices found on this machin: \n";

    /* Open HCI socket  */
    int ctl;
    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
        error("Can't open HCI socket");

    struct hci_dev_list_req *dl;
    if (!(dl = (hci_dev_list_req *) malloc(HCI_MAX_DEV * sizeof(struct hci_dev_req) + sizeof(uint16_t))))
    {
        close(ctl);
        error("Can't allocate memory");
    }

    dl->dev_num = HCI_MAX_DEV;
    struct hci_dev_req *dr = dl->dev_req;

    if (ioctl(ctl, HCIGETDEVLIST, (void *) dl) < 0)
    {
        close(ctl);
        error("Can't get device list");
    }

    // how many devs are up?
    unsigned int upCounter = 0;

    for (int i = 0; i< dl->dev_num; i++)
    {
        struct hci_dev_info di;
        di.dev_id = (dr+i)->dev_id;

        if (ioctl(ctl, HCIGETDEVINFO, (void *) &di) < 0)
            continue;

        const bdaddr_t BDADDR = {0, 0, 0, 0, 0, 0};
        if (hci_test_bit(HCI_RAW, &di.flags) && !bacmp(&di.bdaddr, &BDADDR))
        {
            int dd = hci_open_dev(di.dev_id);
            hci_read_bd_addr(dd, &di.bdaddr, 1000);
            hci_close_dev(dd);
        }

        if (di.dev_id != -1)
        {
            print_dev_info(&di);
            if(!isDown(di.dev_id)) upCounter++;
        }
    }

    if(upCounter == 0)
        std::cout << "** WARNING ** All BT devices are DOWN in this machine! \n\n";
}


void SniffBluetooth::print_dev_info(struct hci_dev_info *di)
{
    char addr[18];
    ba2str(&di->bdaddr, addr);

    printf("%s:\tType: %s  Bus: %s \n",
            di->name,
            hci_typetostr((di->type & 0x30) >> 4),
            hci_bustostr(di->type & 0x0f));

    printf("\tBD Address: %s  ACL MTU: %d:%d  SCO MTU: %d:%d \n",
            addr,
            di->acl_mtu,
            di->acl_pkts,
            di->sco_mtu,
            di->sco_pkts);

    // return the HCI device flags string given its code
    char *str = hci_dflagstostr(di->flags);
    printf("\t%s \n", str);
    bt_free(str);

    struct hci_dev_stats *st = &di->stat;
    printf("\tRX bytes:%d acl:%d sco:%d events:%d errors:%d\n",
            st->byte_rx, st->acl_rx, st->sco_rx, st->evt_rx, st->err_rx);

    printf("\tTX bytes:%d acl:%d sco:%d commands:%d errors:%d\n",
            st->byte_tx, st->acl_tx, st->sco_tx, st->cmd_tx, st->err_tx);

    // print dev features
    printf("\tLMP features: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x \n",
            di->features[0], di->features[1],
            di->features[2], di->features[3],
            di->features[4], di->features[5],
            di->features[6], di->features[7]);

    printf("\tLE support: ");
    std::string featuresStr = lmp_featurestostr(di->features, "\t\t", 63);
    if(featuresStr.find("LE support") != std::string::npos)
        printf("Yes \n");
    else
        printf("No \n");

    printf("\n");
}


void SniffBluetooth::cmd_up(int hdev)
{
    /* Open HCI socket  */
    int ctl;
    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
        error("Can't open HCI socket.");

    /* Start HCI device */
    if (ioctl(ctl, HCIDEVUP, hdev) < 0)
    {
        if (errno == EALREADY)
        {
            close(ctl);
            return;
        }

        error("Can't init device hci%d: %s (%d) \n", hdev, strerror(errno), errno);
    }
}


void SniffBluetooth::cmd_down(int hdev)
{
    /* Open HCI socket  */
    int ctl;
    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
        error("Can't open HCI socket.");

    /* Stop HCI device */
    if (ioctl(ctl, HCIDEVDOWN, hdev) < 0)
    {
        close(ctl);
        error("Can't down device hci%d: %s (%d) \n", hdev, strerror(errno), errno);
    }

    close(ctl);
}


bool SniffBluetooth::isDown(int hdev)
{
    struct hci_dev_info di;
    di.dev_id = hdev;

    if (!hci_test_bit(HCI_UP, &di.flags))
        return true;
    else
        return false;
}


void SniffBluetooth::loadCachedDevices()
{
    std::ifstream infile(cached_BT_devices_filePATH.string().c_str());
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

        auto it = allBTdevices.find(BTaddr);
        if(it == allBTdevices.end())
        {
            BTdevEntry *v = new BTdevEntry(BTname, timeStamp);
            allBTdevices.insert(std::make_pair(BTaddr, *v));
        }

        counter++;
    }

    std::cout << ">>> " << counter << " cached BT devices read from file.";
}


void SniffBluetooth::saveCachedDevices()
{
    if(allBTdevices.empty())
        return;

    FILE *filePtr = fopen (cached_BT_devices_filePATH.string().c_str(), "w");
    if(!filePtr)
        error("can not open file for writing!");

    for(auto &i : allBTdevices)
        fprintf (filePtr, "%s  ,,  %s  ,,  %s \n", i.first.c_str(), i.second.name.c_str(), i.second.timeStamp.c_str());

    fclose(filePtr);
}


// scanNearbyDevices
void SniffBluetooth::scan()
{
    int dev_id = hci_get_route(NULL);  // get the first available Bluetooth adapter
    if(dev_id < 0)
        error("Device is not available");

    int sock = hci_open_dev(dev_id);
    if (sock < 0)
        error("HCI device open failed");

    int len  = 8;       // inquiry lasts for at most 1.28 * len seconds (= 10.24 seconds)
    int max_rsp = 255;  // at most max_rsp devices will be returned
    int flags = IREQ_CACHE_FLUSH; // the cache of previously detected devices is flushed before performing the current inquiry
    inquiry_info *ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    std::cout << std::endl << ">>> Scan for Bluetooth devices... " << std::flush;
    // num_rsp contains the number of discovered devices
    // ii contains the discovered devices info
    int num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);

    if(num_rsp < 0)
    {
        error("Inquiry failed");
    }
    else if(num_rsp == 0)
    {
        std::cout << "Done!" << std::endl;
        std::cout << "No devices found!" << std::endl;
    }
    else if(num_rsp > 0)
    {
        std::cout << "Done!" << std::endl;
        std::cout << num_rsp << " devices found: " << std::endl;

        char addr[19] = { 0 };
        char name[248] = { 0 };
        for (int i = 0; i < num_rsp; i++)
        {
            // get user-friendly name associated with this device
            memset(name, 0, sizeof(name));
            int ret = hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0 /*timeout milliseconds*/);
            // on failure
            if (ret < 0) strcpy(name, "[unknown]");
            printf("    name: %s \n", name);

            // get device address
            ba2str(&(ii+i)->bdaddr, addr);
            printf("    address: %s [mode %d, clkoffset 0x%4.4x] \n", addr, (ii+i)->pscan_rep_mode, btohs((ii+i)->clock_offset));

            std::string comp = companyInfo((ii+i)->bdaddr);
            char oui[9];
            ba2oui(&(ii+i)->bdaddr, oui);
            printf("    OUI company: %s (%s) \n", comp.c_str(), oui);

            // get device class
            std::string dev_class = classInfo((ii+i)->dev_class);
            printf("    device_class: %s \n\n", dev_class.c_str());

            // save device info
            std::string timeDate = currentDateTime();
            auto it = allBTdevices.find(std::string(addr));
            // new BT device
            if(it == allBTdevices.end())
            {
                BTdevEntry *v = new BTdevEntry(std::string(name), timeDate);
                allBTdevices.insert( std::make_pair(std::string(addr), *v) );
            }
            // if BT address already exists, then only update name and timestamp
            else
            {
                it->second.name = std::string(name);
                it->second.timeStamp = timeDate;
            }
        }
    }

    saveCachedDevices();

    free(ii);
    close(sock);
}


// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string SniffBluetooth::currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}


// todo:
std::string SniffBluetooth::companyInfo(bdaddr_t bdaddr)
{
    return "";
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


// check this command: sdptool browse 4C:A5:6D:58:7C:14
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

    std::cout << std::endl << ">>> Service browsing on BT device " << bdaddr << " ... " << std::endl << std::flush;

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
                        printf("    protocol: 0x%04x (%s)", proto, protocolName.c_str());

                        if(proto == RFCOMM_UUID)
                        {
                            int port = sdp_get_proto_port(proto_list, RFCOMM_UUID);
                            printf(", channel: %d \n", port);
                        }
                        else if(proto == L2CAP_UUID)
                        {
                            int psm = sdp_get_proto_port(proto_list, L2CAP_UUID);
                            printf(", psm: %d \n", psm);
                        }
                        else
                            printf("\n");

                        break;
                    }
                }

                // flush what we have so far
                std::cout.flush();
            }
        }
    }

    free(seq);
    sdp_close(session);
}

}
