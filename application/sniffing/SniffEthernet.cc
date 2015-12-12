/****************************************************************************/
/// @file    SniffEthernet.cc
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

#include <SniffEthernet.h>

namespace VENTOS {

Define_Module(VENTOS::SniffEthernet);

SniffEthernet::~SniffEthernet()
{

}


void SniffEthernet::initialize(int stage)
{
    if(stage == 0)
    {
        on = par("on").boolValue();

        if(!on)
            return;

        listInterfaces();
        sniff2();
    }
}


void SniffEthernet::finish()
{
    if(!on)
        return;
}


void SniffEthernet::handleMessage(cMessage *msg)
{

}


void SniffEthernet::listInterfaces()
{
    // get all interfaces
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;

    getifaddrs (&ifap);

    std::cout << std::endl << "List of all interfaces on this machine: " << std::endl;
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            printf("Interface: %-10s  Address: %-20s \n", ifa->ifa_name, addr);
        }
    }

    std::cout << std::endl;

    freeifaddrs(ifap);
}


void SniffEthernet::sniff2()
{
    const char *dev = std::string("eth0").c_str();
    char filter_exp[] = "ip";       /* filter expression [3] */
    int num_packets = 10;           /* number of packets to capture */
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */

    /* print capture info */
    printf("Device: %s\n", dev);
    printf("Number of packets: %d\n", num_packets);
    printf("Filter expression: %s\n", filter_exp);

    /* get network number and mask associated with capture device */
    bpf_u_int32 net;    /* ip address */
    bpf_u_int32 mask;   /* subnet mask */
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1)
    {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
        net = 0;
        mask = 0;
    }

    /* open the device for sniffing.
    pcap_t *pcap_open_live(char *device, int snaplen, int prmisc, int to_ms, char *ebuf)

    snaplen - maximum size of packets to capture in bytes
    promisc - set card in promiscuous mode?
    to_ms   - time to wait for packets in miliseconds before read times out
    errbuf  - if something happens, place error string here

    Note if you change "prmisc" param to anything other than zero, you will
    get all packets your device sees, whether they are intended for you or not!! */
    pcap_t *handle;
    handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
    if (handle == NULL)
        error("Couldn't open device %s: %s\n", dev, errbuf);

    /* make sure we're capturing on an Ethernet device [2] */
    if (pcap_datalink(handle) != DLT_EN10MB)
        error("%s is not an Ethernet", dev);

    /* compile the filter expression */
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1)
        error("Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));

    /* apply the compiled filter */
    if (pcap_setfilter(handle, &fp) == -1)
        error("Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));

    /* now we can set our callback function */
    pcap_loop(handle, num_packets, SniffEthernet::got_packet, NULL);

    /* cleanup */
    pcap_freecode(&fp);
    pcap_close(handle);

    printf("\nCapture complete.\n");
}


/* dissect/print packet */
void SniffEthernet::got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    static int count = 1;  /* packet counter */

    printf("\nPacket number %d:\n", count);
    count++;

    /* define ethernet header */
    const struct sniff_ethernet *ethernet = (struct sniff_ethernet*)(packet);

    /* define/compute ip header offset */
    const struct sniff_ip *ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    int size_ip = IP_HL(ip)*4;
    if (size_ip < 20)
    {
        printf("   * Invalid IP header length: %u bytes\n", size_ip);
        return;
    }

    /* print source and destination IP addresses */
    printf("       From: %s\n", inet_ntoa(ip->ip_src));
    printf("         To: %s\n", inet_ntoa(ip->ip_dst));

    /* determine protocol */
    switch(ip->ip_p)
    {
    case IPPROTO_TCP:
        printf("   Protocol: TCP\n");
        break;
    case IPPROTO_UDP:
        printf("   Protocol: UDP\n");
        return;
    case IPPROTO_ICMP:
        printf("   Protocol: ICMP\n");
        return;
    case IPPROTO_IP:
        printf("   Protocol: IP\n");
        return;
    default:
        printf("   Protocol: unknown\n");
        return;
    }

    /* OK, this packet is TCP */

    /* define/compute tcp header offset */
    const struct sniff_tcp *tcp;
    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    int size_tcp = TH_OFF(tcp)*4;
    if (size_tcp < 20)
    {
        printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
        return;
    }

    printf("   Src port: %d\n", ntohs(tcp->th_sport));
    printf("   Dst port: %d\n", ntohs(tcp->th_dport));

    /* define/compute tcp payload (segment) offset */
    const u_char *payload;
    payload = (const u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    int size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0)
    {
        printf("   Payload (%d bytes):\n", size_payload);
        print_payload(payload, size_payload);
    }
}


/* print packet payload data (avoid printing binary data) */
void SniffEthernet::print_payload(const u_char *payload, int len)
{
    int len_rem = len;
    int line_width = 16;            /* number of bytes per line */
    int line_len;
    int offset = 0;                 /* zero-based offset counter */
    const u_char *ch = payload;

    if (len <= 0)
        return;

    /* data fits on one line */
    if (len <= line_width)
    {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    /* data spans multiple lines */
    for ( ;; )
    {
        /* compute current line length */
        line_len = line_width % len_rem;
        /* print line */
        print_hex_ascii_line(ch, line_len, offset);
        /* compute total remaining */
        len_rem = len_rem - line_len;
        /* shift pointer to remaining bytes to print */
        ch = ch + line_len;
        /* add offset */
        offset = offset + line_width;
        /* check if we have line width chars or less */
        if (len_rem <= line_width)
        {
            /* print last line and get out */
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }
}


/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void SniffEthernet::print_hex_ascii_line(const u_char *payload, int len, int offset)
{
    int i;
    int gap;
    const u_char *ch;

    /* offset */
    printf("%05d   ", offset);

    /* hex */
    ch = payload;
    for(i = 0; i < len; i++)
    {
        printf("%02x ", *ch);
        ch++;
        /* print extra space after 8th byte for visual aid */
        if (i == 7)
            printf(" ");
    }

    /* print space to handle line less than 8 bytes */
    if (len < 8)
        printf(" ");

    /* fill hex gap with spaces if not full line */
    if (len < 16)
    {
        gap = 16 - len;
        for (i = 0; i < gap; i++)
        {
            printf("   ");
        }
    }

    printf("   ");

    /* ascii (if printable) */
    ch = payload;
    for(i = 0; i < len; i++)
    {
        if (isprint(*ch))
            printf("%c", *ch);
        else
            printf(".");
        ch++;
    }

    printf("\n");
}

}
