/****************************************************************************/
/// @file    Bluetooth.cc
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

#include <03_Bluetooth.h>

#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <sys/ioctl.h>
#include <linux/errno.h>

namespace VENTOS {

Define_Module(VENTOS::Bluetooth);

Bluetooth::~Bluetooth()
{

}


void Bluetooth::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to SniffEthernet module
        // we need to call OUITostr
        cModule *module2 = simulation.getSystemModule()->getSubmodule("Ethernet");
        EtherPtr = static_cast<Ethernet *>(module2);
        ASSERT(EtherPtr);

        listLocalDevices = par("listLocalDevices").boolValue();

        // display local devices
        if(listLocalDevices)
        {
            getLocalDevs();
            std::cout.flush();
        }

        BT_on = par("BT_on").boolValue();

        if(!BT_on)
            return;

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        cached_BT_devices_filePATH = VENTOS_FullPath / "results/cached_BT_devices";
    }
}


void Bluetooth::finish()
{


}


void Bluetooth::handleMessage(cMessage *msg)
{


}


void Bluetooth::executeFirstTimeStep()
{

}


void Bluetooth::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (BT_on && !wasExecuted)
    {
        // cached BT devices from previous scans
        loadCachedDevices();

        int dev_id = par("BT_scan_deviceID").longValue();

        if(dev_id == -1)
        {
            // get the first available BT device
            dev_id = hci_get_route(NULL);
            if (dev_id < 0)
                error("Device is not available");
        }

        int scanLength = par("BT_scan_length").longValue();

        // looking for nearby BT devices
        scan(dev_id, scanLength);

        // request for service
        serviceDiscovery("30:75:12:6D:B2:3A");
        serviceDiscovery("4C:A5:6D:58:7C:14");

        wasExecuted = true;
    }
}


void Bluetooth::getLocalDevs()
{
    std::cout << std::endl << ">>> Local Bluetooth devices on this machine: \n";

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

    // count how many BT devices
    unsigned int devCounter = 0;
    // count how many BT devices are UP ?
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
            devCounter++;
            print_dev_info(&di);

            if(!isDown(di.dev_id))
                upCounter++;
        }
    }

    if(devCounter == 0)
        std::cout << "No devices found! \n\n";

    if(devCounter > 0 && upCounter == 0)
    {
        std::cout << "** WARNING ** All BT devices are DOWN! \n";
        std::cout << "Use 'sudo hciconfig <device> up' to enable Bluetooth device. \n\n";
    }
}


void Bluetooth::print_dev_info(struct hci_dev_info *di)
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

    // show OUI lookup name
    const u_int8_t BTaddr[3] = {(&di->bdaddr)->b[5], (&di->bdaddr)->b[4], (&di->bdaddr)->b[3]};
    std::string OUI = EtherPtr->OUITostr(BTaddr);
    printf("\tOUI: %s \n", OUI.c_str());

    // return the HCI device flags string given its code
    char *str = hci_dflagstostr(di->flags);
    printf("\t%s \n", str);

    //    /* HCI dev flags mapping */
    //    { "UP",      HCI_UP      },
    //    { "INIT",    HCI_INIT    },
    //    { "RUNNING", HCI_RUNNING },
    //    { "RAW",     HCI_RAW     },
    //    { "PSCAN",   HCI_PSCAN   },
    //    { "ISCAN",   HCI_ISCAN   },
    //    { "INQUIRY", HCI_INQUIRY },
    //    { "AUTH",    HCI_AUTH    },
    //    { "ENCRYPT", HCI_ENCRYPT },

    printf("\tDiscovarable: ");
    std::string status = str;
    if(status.find("PSCAN") != std::string::npos && status.find("ISCAN") != std::string::npos)
        printf("Yes \n");
    else
        printf("No \n");

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

    printf("\tBLE support: ");
    std::string featuresStr = lmp_featurestostr(di->features, "\t\t", 63);
    if(featuresStr.find("LE support") != std::string::npos)
        printf("Yes \n");
    else
        printf("No \n");

    if (hci_test_bit(HCI_UP, &di->flags))
    {
        // get name
        std::string name = cmd_name(di->dev_id);
        printf("\tName: %s \n", name.c_str());

        // get class
        std::string dev_class = cmd_class(di->dev_id);
        printf("\tDevice class: %s \n", dev_class.c_str());

        // get manufacture
        std::string version = cmd_company(di->dev_id);
        printf("\tManufacturer: %s \n", version.c_str());
    }

    printf("\n");
}


std::string Bluetooth::cmd_name(int dev_id)
{
    // open device
    int dd = hci_open_dev(dev_id);
    if(dd < 0)
        error("Can not open device!");

    char name[249];

    if (hci_read_local_name(dd, sizeof(name), name, 1000) < 0)
        error("Can't read local name: %s (%d)\n", strerror(errno), errno);

    for (int i = 0; i < 248 && name[i]; i++)
    {
        if ((unsigned char) name[i] < 32 || name[i] == 127)
            name[i] = '.';
    }

    name[248] = '\0';

    hci_close_dev(dd);

    return std::string(name);
}


std::string Bluetooth::cmd_class(int dev_id)
{
    // open device
    int dd = hci_open_dev(dev_id);
    if(dd < 0)
        error("Can not open device!");

    // get device class
    uint8_t cls[3];
    if (hci_read_class_of_dev(dd, cls, 1000) < 0)
        error("Can't read class of device on hci%d: %s (%d) \n", dev_id, strerror(errno), errno);

    hci_close_dev(dd);

    return cmd_class(cls);
}


// Decode device class
// from https://www.bluetooth.com/specifications/assigned-numbers/baseband
std::string Bluetooth::cmd_class(uint8_t dev_class[3])
{
    int flags = dev_class[2];
    int majorNum = dev_class[1];
    int minorNum = dev_class[0] >> 2;

    std::ostringstream os;
    os.str("");

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


std::string Bluetooth::cmd_company(int dev_id)
{
    // open device
    int dd = hci_open_dev(dev_id);
    if(dd < 0)
        error("Can not open device!");

    struct hci_version ver;
    if (hci_read_local_version(dd, &ver, 1000) < 0)
        error("Can't read version info: %s (%d) \n", strerror(errno), errno);

    char buffer[200];
    sprintf (buffer, "%s (%d)", bt_compidtostr(ver.manufacturer), ver.manufacturer);

    hci_close_dev(dd);

    return buffer;
}


void Bluetooth::cmd_up(int hdev)
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


void Bluetooth::cmd_down(int hdev)
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


bool Bluetooth::isDown(int hdev)
{
    int ctl;
    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
        error("Can't open HCI socket.");

    struct hci_dev_info di;
    di.dev_id = hdev;

    if (ioctl(ctl, HCIGETDEVINFO, (void *) &di) < 0)
        error("can not get dev info!");

    if (!hci_test_bit(HCI_UP, &di.flags))
        return true;
    else
        return false;
}


void Bluetooth::piscan(int hdev, std::string scan)
{
    /* Open HCI socket  */
    int ctl;
    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
        error("Can't open HCI socket");

    struct hci_dev_req dr;
    dr.dev_id  = hdev;

    if(scan == "noscan")
        dr.dev_opt = SCAN_DISABLED;             // Disable scan
    else if(scan == "iscan")
        dr.dev_opt = SCAN_INQUIRY;              // Enable Inquiry scan
    else if(scan == "pscan")
        dr.dev_opt = SCAN_PAGE;                 // Enable Page scan
    else if(scan == "piscan")
        dr.dev_opt = SCAN_PAGE | SCAN_INQUIRY;  // Enable Page and Inquiry scan

    if (ioctl(ctl, HCISETSCAN, (unsigned long) &dr) < 0)
        error("Can't set scan mode on hci%d: %s (%d) \n", hdev, strerror(errno), errno);
}


void Bluetooth::loadCachedDevices()
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


void Bluetooth::saveCachedDevices()
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


// scan nearby BT devices
void Bluetooth::scan(int dev_id, int len)
{
    int sock = hci_open_dev(dev_id);
    if (sock < 0)
        error("HCI device open failed");

    int max_rsp = 255;  // at most max_rsp devices will be returned
    int flags = IREQ_CACHE_FLUSH; // the cache of previously detected devices is flushed before performing the current inquiry
    inquiry_info *ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    // inquiry lasts for at most 1.28 * len seconds (= 10.24 seconds)
    int scanTime = round(len * 1.28);
    std::cout << std::endl << ">>> Scanning Bluetooth devices on hci" << dev_id << " for " << scanTime << " seconds... \n" << std::flush;

    // num_rsp contains the number of discovered devices
    // ii contains the discovered devices info
    int num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);

    if(num_rsp < 0)
    {
        error("Inquiry failed");
    }
    else if(num_rsp == 0)
    {
        std::cout << "No devices found!" << std::endl;
        std::cout.flush();
    }
    else if(num_rsp > 0)
    {
        std::cout << num_rsp << " devices found: " << std::endl;
        std::cout.flush();

        char addr[19] = { 0 };
        char name[248] = { 0 };
        for (int i = 0; i < num_rsp; i++)
        {
            // get user-friendly name associated with this device
            memset(name, 0, sizeof(name));
            int ret = hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0 /*timeout milliseconds*/);
            // on failure
            if (ret < 0) strcpy(name, "unknown");
            printf("    Name: %s \n", name);

            // get device address
            ba2str(&(ii+i)->bdaddr, addr);
            printf("    BD Address: %s [mode %d, clkoffset 0x%4.4x] \n", addr, (ii+i)->pscan_rep_mode, btohs((ii+i)->clock_offset));

            // show Manufacturer
            const u_int8_t BTaddr[3] = {(&(ii+i)->bdaddr)->b[5], (&(ii+i)->bdaddr)->b[4], (&(ii+i)->bdaddr)->b[3]};
            std::string OUI = EtherPtr->OUITostr(BTaddr);
            printf("    OUI: %s \n", OUI.c_str());

            // get device class
            std::string dev_class = cmd_class((ii+i)->dev_class);
            printf("    Device class: %s \n\n", dev_class.c_str());

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

            // flush what we have so far
            std::cout.flush();
        }
    }

    saveCachedDevices();

    free(ii);
    hci_close_dev(sock);
}


// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string Bluetooth::currentDateTime()
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


// check this command: sdptool browse 4C:A5:6D:58:7C:14
void Bluetooth::serviceDiscovery(std::string bdaddr, uint16_t UUID)
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
            }

            // flush what we have so far
            std::cout.flush();
        }
    }

    free(seq);
    sdp_close(session);
}


void Bluetooth::cmd_cmd(int dev_id, uint8_t ogf, uint16_t ocf, std::string payload)
{
    if (dev_id < 0)
        error("Not a valid device");

    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Device open failed");

    if(payload == "")
        error("payload is empty!");

    std::cout << std::endl << ">>> Sending command on hci" << dev_id << "... \n" << std::flush;

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


void Bluetooth::hex_dump(std::string pref, int width, unsigned char *buf, int len)
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
