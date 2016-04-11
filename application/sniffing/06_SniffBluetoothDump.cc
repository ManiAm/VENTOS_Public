/****************************************************************************/
/// @file    SniffBluetoothDump.cc
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

#include <06_SniffBluetoothDump.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

namespace VENTOS {

#define SNAP_LEN_BT  HCI_MAX_FRAME_SIZE
#define DUMP_WIDTH   20

struct hcidump_hdr {
    uint16_t    len;
    uint8_t     in;
    uint8_t     pad;
    uint32_t    ts_sec;
    uint32_t    ts_usec;
} __attribute__ ((packed));

#define HCIDUMP_HDR_SIZE (sizeof(struct hcidump_hdr))

struct frame {
    void        *data;
    uint32_t    data_len;
    void        *ptr;
    uint32_t    len;
    uint16_t    dev_id;
    uint8_t     in;
    uint8_t     master;
    uint16_t    handle;
    uint16_t    cid;
    uint16_t    num;
    uint8_t     dlci;
    uint8_t     channel;
    unsigned long   flags;
    struct timeval  ts;
    int     pppdump_fd;
    int     audio_fd;
};

Define_Module(VENTOS::SniffBluetoothDump);

SniffBluetoothDump::~SniffBluetoothDump()
{

}


void SniffBluetoothDump::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        // register signals
        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        dump_On = par("dump_On").boolValue();
    }
}


void SniffBluetoothDump::finish()
{
    super::finish();

}


void SniffBluetoothDump::handleMessage(cMessage *msg)
{
    super::handleMessage(msg);

}


void SniffBluetoothDump::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        SniffBluetoothDump::executeEachTimestep();
    }
    else if(signalID == Signal_executeFirstTS)
    {
        SniffBluetoothDump::executeFirstTimeStep();
    }
}


void SniffBluetoothDump::executeFirstTimeStep()
{
    super::executeFirstTimeStep();

}


void SniffBluetoothDump::executeEachTimestep()
{
    super::executeEachTimestep();

    // run this code only once
    static bool wasExecuted = false;
    if (dump_On && !wasExecuted)
    {
        int dev_id = par("BLE_dump_deviceID").longValue();

        if(dev_id == -1)
        {
            // get the first available BT device
            dev_id = hci_get_route(NULL);
            if (dev_id < 0)
                error("Device is not available");
        }

        int scan_type = par("BLE_scan_type").longValue();
        uint16_t interval = par("BLE_interval").longValue();
        uint16_t window = par("BLE_window").longValue();
        uint8_t own_type = par("BLE_own_type").longValue();
        uint8_t filter_policy = par("BLE_filter_policy").longValue();
        lescanEnable(dev_id, scan_type, interval, window, own_type, filter_policy);

        int sock = open_socket(dev_id);
        int timeout = par("sniff_time").longValue();
        process_frames(dev_id, sock, timeout);

        // disable LE scan
        lescanDisable(dev_id);
        // close the socket
        close(sock);

        wasExecuted = true;
    }
}


void SniffBluetoothDump::lescanEnable(int dev_id, uint8_t scan_type, uint16_t interval, uint16_t window, uint8_t own_type, uint8_t filter_policy)
{
    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Can't open device");

    std::cout << ">>> Enabling BLE scan on hci" << dev_id << " ... \n";
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
        error("Set scan parameters failed: %s", strerror(errno));
    }

    uint8_t filter_dup = 0x00;  // do not filter duplicates
    err = hci_le_set_scan_enable(dd, 0x01 /*enable*/, filter_dup, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        error("Enable scan failed");
    }

    hci_close_dev(dd);
}


void SniffBluetoothDump::lescanDisable(int dev_id)
{
    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Can't open device");

    uint8_t filter_dup = 0x00;  // do not filter duplicates
    int err = hci_le_set_scan_enable(dd, 0x00 /*disable*/, filter_dup, 1000 /*timeout*/);
    if (err < 0)
    {
        hci_close_dev(dd);
        error("Disable scan failed");
    }

    hci_close_dev(dd);
}


int SniffBluetoothDump::open_socket(int dev_id)
{
    int dd = hci_open_dev(dev_id);
    if (dd < 0)
        error("Can't open device");

    struct hci_dev_info di;
    if (hci_devinfo(dev_id, &di) < 0)
        error("Can't get device info");

    int opt = hci_test_bit(HCI_RAW, &di.flags);
    if (ioctl(dd, HCISETRAW, opt) < 0)
    {
        if (errno == EACCES)
            error("Can't access device");
    }

    hci_close_dev(dd);

    /* Create HCI socket */
    int sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (sk < 0)
        error("Can't create raw socket");

    opt = 1;
    if (setsockopt(sk, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0)
        error("Can't enable data direction info");

    opt = 1;
    if (setsockopt(sk, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0)
        error("Can't enable time stamp");

    /* Setup filter */
    struct hci_filter flt;
    hci_filter_clear(&flt);
    hci_filter_all_ptypes(&flt);
    hci_filter_all_events(&flt);
    if (setsockopt(sk, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
        error("Can't set filter");

    /* Bind socket to the HCI device */
    struct sockaddr_hci addr;
    memset(&addr, 0, sizeof(addr));
    addr.hci_family = AF_BLUETOOTH;
    addr.hci_dev = dev_id;
    if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        error("Can't attach to device hci%d. %s(%d) \n", dev_id, strerror(errno), errno);

    if(sk < 0)
        error("Socket number is invalid");

    return sk;
}


void SniffBluetoothDump::process_frames(int dev_id, int sock, int timeout)
{
    int hdr_size = HCIDUMP_HDR_SIZE;

    int snap_len = SNAP_LEN_BT;
    char *buf = (char *) malloc(snap_len + hdr_size);
    if (!buf)
        error("Can't allocate data buffer");

    struct frame frm;
    frm.data = buf + hdr_size;

    char *ctrl = (char *) malloc(100);
    if (!ctrl)
    {
        free(buf);
        error("Can't allocate control buffer");
    }

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));

    int nfds = 0;
    struct pollfd fds[2];
    fds[nfds].fd = sock;
    fds[nfds].events = POLLIN;
    fds[nfds].revents = 0;

    nfds++;

    std::cout << std::endl;
    printf(">>> Start sniffing hci%d for %d seconds (snap_len is %d)...  \n\n", dev_id, timeout, snap_len);
    std::cout.flush();

    int ts = time(NULL);  // get current time

    while (1)
    {
        int n = poll(fds, nfds, timeout);
        if (n <= 0)
        {
            // should we terminate sniffing?
            int elapsed = time(NULL) - ts;
            if (timeout != -1 && elapsed >= timeout)
                break;

            continue;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                if (fds[i].fd == sock)
                    printf("Device disconnected \n");
                else
                    printf("Client disconnect \n");

                return;
            }
        }

        struct iovec  iv = {};
        iv.iov_base = frm.data;
        iv.iov_len  = snap_len;

        msg.msg_iov = &iv;
        msg.msg_iovlen = 1;
        msg.msg_control = ctrl;
        msg.msg_controllen = 100;

        int len = recvmsg(sock, &msg, MSG_DONTWAIT);
        if (len < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;

            error("Receive failed");
        }

        /* Process control message */
        frm.data_len = len;
        frm.dev_id = dev_id;
        frm.in = 0;
        frm.pppdump_fd = -1;
        frm.audio_fd = -1;

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        while (cmsg)
        {
            int dir;
            switch (cmsg->cmsg_type)
            {
            case HCI_CMSG_DIR:
                memcpy(&dir, CMSG_DATA(cmsg), sizeof(int));
                frm.in = (uint8_t) dir;
                break;
            case HCI_CMSG_TSTAMP:
                memcpy(&frm.ts, CMSG_DATA(cmsg), sizeof(struct timeval));
                break;
            }

            cmsg = CMSG_NXTHDR(&msg, cmsg);
        }

        frm.ptr = frm.data;
        frm.len = frm.data_len;

        // hex dump
        hex_dump(&frm);

        // decode packet
        hci_dump(&frm);

        std::cout << std::endl;

        // flush what we have so far
        std::cout.flush();

        int elapsed = time(NULL) - ts;
        if (timeout != -1 && elapsed >= timeout)
            break;
    }
}


/* Sample output

04 3E 2A 02 01 03 00 '30 CD DD DC 84 0C'
'1E' '02 01 1A' '1A FF' '4C 00' '02 15' 'E2 C5 6D B5 DF FB 48 D2 B0 60 D0 F5 A7 10 96 E0' '00 02' '00 01' 'C5 C5'

 */
void SniffBluetoothDump::hex_dump(struct frame *frm)
{
    if (!frm->len)
        return;

    /* convert timestamp to readable format */
    struct tm tm;
    time_t t = frm->ts.tv_sec;
    localtime_r(&t, &tm);
    printf("%04d-%02d-%02d %02d:%02d:%02d.%06lu",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, frm->ts.tv_usec);

    printf(", Channel: %d \n", frm->channel);

    unsigned int i, n;
    unsigned char *buf = (unsigned char *)frm->ptr;
    for (i = 0, n = 1; i < frm->len; i++, n++)
    {
        printf("%2.2X ", buf[i]);

        if (n == DUMP_WIDTH)
        {
            printf("\n");
            n = 0;
        }
    }

    if (i && n != 1)
        printf("\n");
}


void SniffBluetoothDump::hci_dump(struct frame *frm)
{
    uint8_t type = *(uint8_t *)frm->ptr;

    frm->ptr = (unsigned char *)frm->ptr + 1;
    frm->len--;

    switch (type)
    {
    case HCI_COMMAND_PKT:
        //command_dump(0, frm);
        printf("HCI_COMMAND_PKT \n");
        break;

    case HCI_EVENT_PKT:
        //event_dump(0, frm);
        printf("HCI_EVENT_PKT \n");
        break;

    case HCI_ACLDATA_PKT:
        //acl_dump(0, frm);
        printf("HCI_ACLDATA_PKT \n");
        break;

    case HCI_SCODATA_PKT:
        //sco_dump(0, frm);
        printf("HCI_SCODATA_PKT \n");
        break;

    case HCI_VENDOR_PKT:
        //vendor_dump(0, frm);
        printf("HCI_VENDOR_PKT \n");
        break;

    default:
        printf("Unknown: type 0x%2.2x len %d\n", type, frm->len);
        break;
    }
}

}
