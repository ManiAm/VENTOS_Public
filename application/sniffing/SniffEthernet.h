/****************************************************************************/
/// @file    SniffEthernet.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Dec 2015
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

#ifndef SNIFFETHERNET
#define SNIFFETHERNET

#include <BaseApplLayer.h>
#include <Appl.h>
#include "TraCI_Extend.h"
#include "pcap.h"

#include <netinet/in.h>
#include <netinet/ip.h>       // iphdr
#include <netinet/ip6.h>      // ipv6hdr
#include <netinet/tcp.h>      // tcphdr
#include <netinet/udp.h>      // udphdr
#include <netinet/ip_icmp.h>  // icmphdr

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/ethernet.h>  // ether_header, ETHER_ADDR_LEN, ETHER_HDR_LEN


#include <netdb.h>      // Needed for the socket functions

namespace VENTOS {

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

class SniffEthernet : public BaseApplLayer
{
public:
    virtual ~SniffEthernet();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

private:
    void getOUI();
    void getPortNumbers();
    void listInterfaces();
    void startSniffing();

    static void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
    static const char * formatMACaddress(const unsigned char MACData[]);
    static const char* MACtoOUI(const unsigned char MACData[]);
    static void processIPv4(const u_char *packet);
    static void processIPv6(const u_char *packet);
    static const char* portToApplication(int port);
    static void processTCP(const u_char *packet, const struct iphdr *ip);
    static void processUDP(const u_char *packet, const struct iphdr *ip);
    static void processICMP(const u_char *packet, const struct iphdr *ip);
    static void print_dataPayload(const u_char *payload, int len);
    static void print_hex_ascii_line(const u_char *payload, int len, int offset);

private:
    // NED variables
    bool on;
    std::string interface;
    std::string filter_exp;
    int num_packets;
    static bool printDataPayload;

    // variables
    TraCI_Extend *TraCI;
    std::vector<std::string> allDev;
    static std::map<std::string, std::string> OUI;
    static std::map<int, std::string> portNumber;
};

}

#endif
