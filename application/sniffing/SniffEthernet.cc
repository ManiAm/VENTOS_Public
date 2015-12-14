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
#include "boost/format.hpp"

namespace VENTOS {

bool SniffEthernet::printDataPayload;
std::map<std::string, std::string> SniffEthernet::OUI;
std::map<int, std::string> SniffEthernet::portNumber;

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

        interface = par("interface").stdstringValue();

        // check if interface is valid!
        if(std::find(allDev.begin(), allDev.end(), interface.c_str()) == allDev.end())
            error("%s is not a valid interface!", interface.c_str());

        filter_exp = par("filter_exp").stdstringValue(); /* filter expression [3] */
        num_packets = par("num_packets").longValue();    /* number of packets to capture.
                                                            -1: loop forever and call got_packet for every received packet  */

        printDataPayload = par("printDataPayload").boolValue();

        getOUI();
        getPortNumbers();
        startSniffing();
    }
}


void SniffEthernet::finish()
{

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

            // add interface name
            allDev.push_back(ifa->ifa_name);
        }
    }

    freeifaddrs(ifap);

    std::cout.flush();
}


const char * SniffEthernet::formatTime(struct timeval ts)
{
    int hour = (long long)ts.tv_sec / 100000000;
    int remain = (long long)ts.tv_sec % 100000000;
    int min = remain / 1000000;
    remain = remain % 1000000;
    int sec = remain / 10000;

    char *buffer = new char[60];
    sprintf(buffer, "%d:%d:%d.%-6ld", hour, min, sec, ts.tv_usec);

    return buffer;
}


void SniffEthernet::getOUI()
{
    // OUI is downloaded from https://www.wireshark.org/tools/oui-lookup.html
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path manuf_FullPath = VENTOS_FullPath / "sniff_manuf";

    std::ifstream in(manuf_FullPath.string().c_str());
    if(in == NULL)
        error("cannot open file sniff_manuf at %s", manuf_FullPath.string().c_str());

    std::string line;
    while(getline(in, line))
    {
        std::vector<std::string> lineToken = cStringTokenizer(line.c_str()).asVector();

        if(lineToken.size() < 2)
            continue;

        auto it = OUI.find(lineToken[0]);
        if(it == OUI.end())
            OUI[lineToken[0]] = lineToken[1];
    }
}


const char * SniffEthernet::MACtoOUI(const unsigned char MACData[])
{
    boost::format fmt("%02X:%02X:%02X");

    for (int i = 0; i != 3; ++i)
        fmt % static_cast<unsigned int>(MACData[i]);

    std::string firstThreeOctet =  fmt.str();

    auto it = OUI.find(firstThreeOctet);
    if(it != OUI.end())
        return (it->second).c_str();
    else
        return "?";
}


const char * SniffEthernet::formatMACaddress(const unsigned char MACData[])
{
    boost::format fmt("%02X:%02X:%02X:%02X:%02X:%02X");

    for (int i = 0; i != 6; ++i)
        fmt % static_cast<unsigned int>(MACData[i]);

    return fmt.str().c_str();
}


const char * SniffEthernet::formatIPaddressMAC(const unsigned char addr[])
{
    boost::format fmt("%u.%u.%u.%u");

    for(int i = 0; i < 4; ++i)
        fmt % static_cast<unsigned int>(addr[i]);

    return fmt.str().c_str();
}


const char * SniffEthernet::formatIPaddress(u_int32_t addr)
{
    in_addr srcAdd;
    srcAdd.s_addr = addr;

    return inet_ntoa(srcAdd);
}


void SniffEthernet::getPortNumbers()
{
    // services files in Linux is in /etc/services
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path services_FullPath = VENTOS_FullPath / "sniff_services";

    std::ifstream in(services_FullPath.string().c_str());
    if(in == NULL)
        error("cannot open file sniff_services at %s", services_FullPath.string().c_str());

    std::string line;
    while(getline(in, line))
    {
        std::vector<std::string> lineToken = cStringTokenizer(line.c_str()).asVector();

        if(lineToken.size() < 2)
            continue;

        std::vector<std::string> port_prot = cStringTokenizer(lineToken[1].c_str(), "/").asVector();

        if(port_prot.size() != 2)
            continue;

        try
        {
            auto it = portNumber.find(std::stoi(port_prot[0]));
            if(it == portNumber.end())
                portNumber[std::stoi(port_prot[0])] = lineToken[0];
        }
        catch(std::exception e)
        {
            // do nothing!
        }
    }
}


const char * SniffEthernet::portToApplication(int port)
{
    auto it = portNumber.find(port);

    if(it != portNumber.end())
        return (it->second).c_str();
    else return "?";
}


void SniffEthernet::startSniffing()
{
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */

    /* get network number and mask associated with capture device */
    bpf_u_int32 net;    /* ip address */
    bpf_u_int32 mask;   /* subnet mask */
    if (pcap_lookupnet(interface.c_str(), &net, &mask, errbuf) == -1)
    {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", interface.c_str(), errbuf);
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
    handle = pcap_open_live(interface.c_str(), SNAP_LEN, 1, 1000, errbuf);
    if (handle == NULL)
        error("Couldn't open device %s: %s\n", interface.c_str(), errbuf);

    /* make sure we're capturing on an Ethernet device [2] */
    if (pcap_datalink(handle) != DLT_EN10MB)
        error("%s is not an Ethernet", interface.c_str());

    /* compile the filter expression */
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, filter_exp.c_str(), 0, net) == -1)
        error("Couldn't parse filter %s: %s\n", filter_exp.c_str(), pcap_geterr(handle));

    /* apply the compiled filter */
    if (pcap_setfilter(handle, &fp) == -1)
        error("Couldn't install filter %s: %s\n", filter_exp.c_str(), pcap_geterr(handle));

    // flush everything we have before entering loop
    std::cout.flush();

    /* now we can set our callback function */
    pcap_loop(handle, num_packets, SniffEthernet::got_packet, NULL);

    /* cleanup */
    pcap_freecode(&fp);
    pcap_close(handle);

    printf("\nCapture complete.\n");
}


void SniffEthernet::got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    static int count = 1;  /* packet counter */

    printf("\n#%-3d %s \n", count, formatTime(header->ts));
    count++;

    printf("Ethernet Frame --> ");

    /* define Ethernet header */
    const struct ether_header *p = (struct ether_header*)(packet);

    /* what packet type we have..*/
    if (ntohs (p->ether_type) == ETHERTYPE_ARP)
    {
        printf("Src MAC: %s (%s), ", formatMACaddress(p->ether_shost), MACtoOUI(p->ether_shost));
        printf("Dst MAC: %s (%s), ", formatMACaddress(p->ether_dhost), MACtoOUI(p->ether_dhost));
        printf("Type: ETHERTYPE_ARP \n");

        processARP(packet);
        std::cout.flush();
        return;
    }
    else if (ntohs (p->ether_type) == ETHERTYPE_IP)
    {
        printf("Src MAC: %s (%s), ", formatMACaddress(p->ether_shost), MACtoOUI(p->ether_shost));
        printf("Dst MAC: %s (%s), ", formatMACaddress(p->ether_dhost), MACtoOUI(p->ether_dhost));
        printf("Type: ETHERTYPE_IP \n");
    }
    else  if (ntohs (p->ether_type) == ETHERTYPE_IPV6)
    {
        printf("Src MAC: %s (%s), ", formatMACaddress(p->ether_shost), MACtoOUI(p->ether_shost));
        printf("Dst MAC: %s (%s), ", formatMACaddress(p->ether_dhost), MACtoOUI(p->ether_dhost));
        printf("Type: ETHERTYPE_IPV6 \n");
    }
    else
    {
        printf("Invalid Ethernet type %x", ntohs(p->ether_type));
        return;
    }

    const struct iphdr *ip = (struct iphdr*)(packet + ETHER_HDR_LEN);

    if(ip->version == 4)
        processIPv4(packet);
    else if(ip->version == 6)
        processIPv6(packet);
    else
    {
        printf("    Invalid IP version number: %u ", ip->version);
        return;
    }

    std::cout.flush();
}


void SniffEthernet::processARP(const u_char *packet)
{
    const struct ether_arp *arp = (struct ether_arp*)(packet + ETHER_HDR_LEN);

    printf("    ARP message --> ");

    // print header information
    printf("HrwType: %u, ", ntohs(arp->ea_hdr.ar_hrd));
    printf("ProtocolType: 0x%02X, ", ntohs(arp->ea_hdr.ar_pro));
    printf("HrwLen: %u, ", arp->ea_hdr.ar_hln);
    printf("Protocol len: %u, ", arp->ea_hdr.ar_pln);
    printf("Opcode: %u, \n", ntohs(arp->ea_hdr.ar_op));

    // print payload information
    printf("                    ");
    printf("SenderHrwAdd: %s, ", formatMACaddress(arp->arp_sha));
    printf("SenderProtAdd: %s \n", formatIPaddressMAC(arp->arp_spa));
    printf("                    ");
    printf("TargetHrwAdd: %s, ", formatMACaddress(arp->arp_tha));
    printf("TargetProtAdd: %s", formatIPaddressMAC(arp->arp_tpa));
}


void SniffEthernet::processIPv4(const u_char *packet)
{
    const struct iphdr *ip = (struct iphdr*)(packet + ETHER_HDR_LEN);

    int size_ip = ip->ihl * 4;  // size in byte
    if (size_ip < 20) // minimum ipv4 header size is 20 bytes
    {
        printf("    Invalid IPv4 header length: %u bytes", size_ip);
        return;
    }

    printf("    IPv4 packet --> ");

    /* print source and destination IP addresses */
    printf("From: %s, ", formatIPaddress(ip->saddr));
    printf("To: %s, ", formatIPaddress(ip->daddr));

    /* determine protocol */
    switch(ip->protocol)
    {
    case IPPROTO_TCP:
        printf("Protocol: %s", "TCP");
        processTCP(packet, ip);
        break;
    case IPPROTO_UDP:
        printf("Protocol: %s", "UDP");
        processUDP(packet, ip);
        break;
    case IPPROTO_ICMP:
        printf("Protocol: %s", "ICMP");
        processICMP(packet, ip);
        break;
    default:
        printf("Protocol: %s", "?");
        break;
    }
}


void SniffEthernet::processIPv6(const u_char *packet)
{
    // const struct ip6_hdr *ipv6 = (struct ip6_hdr*)(packet + ETHER_HDR_LEN);

    // IPv6 header is always 40 bytes long

}


void SniffEthernet::processTCP(const u_char *packet, const struct iphdr *ip)
{
    /* compute tcp header offset */
    int size_ip = ip->ihl * 4;  // size in byte
    const struct tcphdr *tcp = (struct tcphdr*)(packet + ETHER_HDR_LEN + size_ip);

    int size_tcp = tcp->th_off * 4;
    if (size_tcp < 20)
    {
        printf("\n        Invalid TCP header length: %u bytes, ", size_tcp);
        return;
    }

    printf("\n        TCP segment --> ");

    printf("Src port: %d (%s), ", ntohs(tcp->th_sport), portToApplication(ntohs(tcp->th_sport)));
    printf("Dst port: %d (%s), ", ntohs(tcp->th_dport), portToApplication(ntohs(tcp->th_dport)));

    /* compute tcp payload (segment) offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    int size_payload = ntohs(ip->tot_len) - (size_ip + size_tcp);
    printf("Payload (%d bytes)", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


void SniffEthernet::processUDP(const u_char *packet, const struct iphdr *ip)
{
    /* compute udp header offset */
    int size_ip = ip->ihl * 4;  // size in byte
    const struct udphdr *udp = (struct udphdr*)(packet + ETHER_HDR_LEN + size_ip);

    int size_udp = udp->len;  // size in byte
    if (size_udp < 8)
    {
        printf("\n        Invalid UDP length: %u bytes, ", size_udp);
        return;
    }

    printf("\n        UDP datagram --> ");

    printf("Src port: %d (%s), ", ntohs(udp->uh_sport), portToApplication(ntohs(udp->uh_sport)));
    printf("Dst port: %d (%s), ", ntohs(udp->uh_dport), portToApplication(ntohs(udp->uh_dport)));

    /* compute udp payload offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + 8);

    /* compute udp payload size */
    int size_payload = ntohs(ip->tot_len) - (size_ip + 8);
    printf("Payload (%d bytes)", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


void SniffEthernet::processICMP(const u_char *packet, const struct iphdr *ip)
{
    /* compute icmp header offset */
    int size_ip = ip->ihl * 4;  // size in byte
    const struct icmphdr *icmp = (struct icmphdr*)(packet + ETHER_HDR_LEN + size_ip);

    printf("\n        ICMP message --> ");

    printf("Type: %d, ", icmp->type);
    printf("Code: %d, ", icmp->code);

    /* compute icmp payload offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + 8);

    /* compute icmp payload size */
    int size_payload = ntohs(ip->tot_len) - (size_ip + 8);
    printf("Payload (%d bytes)", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


/* print packet payload data (avoid printing binary data) */
void SniffEthernet::print_dataPayload(const u_char *payload, int len)
{
    // double-check again
    if (len <= 0)
        return;

    printf("\n");

    int line_width = 16;   /* number of bytes per line */
    int offset = 0;        /* zero-based offset counter */
    const u_char *ch = payload;

    /* data fits on one line */
    if (len <= line_width)
    {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    int len_rem = len;
    int line_len;

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
    printf("            %05d   ", offset);

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
