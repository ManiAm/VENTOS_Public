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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

namespace VENTOS {

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN  6

/* Ethernet header */
struct sniff_ethernet
{
    u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
    u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
    u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip
{
    u_char  ip_vhl;                   /* version << 4 | header length >> 2 */
    u_char  ip_tos;                   /* type of service */
    u_short ip_len;                   /* total length */
    u_short ip_id;                    /* identification */
    u_short ip_off;                   /* fragment offset field */
#define IP_RF 0x8000                  /* reserved fragment flag */
#define IP_DF 0x4000                  /* dont fragment flag */
#define IP_MF 0x2000                  /* more fragments flag */
#define IP_OFFMASK 0x1fff             /* mask for fragmenting bits */
    u_char  ip_ttl;                   /* time to live */
    u_char  ip_p;                     /* protocol */
    u_short ip_sum;                   /* checksum */
    struct  in_addr ip_src, ip_dst;   /* source and dest address */
};

#define IP_HL(ip)   (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)    (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp
{
    u_short th_sport;               /* source port */
    u_short th_dport;               /* destination port */
    tcp_seq th_seq;                 /* sequence number */
    tcp_seq th_ack;                 /* acknowledgement number */
    u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
    u_char  th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;                 /* window */
    u_short th_sum;                 /* checksum */
    u_short th_urp;                 /* urgent pointer */
};

class SniffEthernet : public BaseApplLayer
{
public:
    virtual ~SniffEthernet();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

private:
    void listInterfaces();
    void sniff2();
    static void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
    static void print_payload(const u_char *payload, int len);
    static void print_hex_ascii_line(const u_char *payload, int len, int offset);

private:
    TraCI_Extend *TraCI;
    bool on;
};

}

#endif
