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
#include <fstream>
#include "boost/format.hpp"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

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

        interface = par("interface").stdstringValue();
        filter_exp = par("filter_exp").stdstringValue();
        printCaptured = par("printCaptured").boolValue();
        printDataPayload = par("printDataPayload").boolValue();
        printStat = par("printStat").boolValue();

        // register signals
        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        pcap_handle = NULL;

        captureEvent = new cMessage("captureEvent", KIND_TIMER);

        listInterfaces();
        getOUIFromFile();
        getPortNumbersFromFile();
    }
}


void SniffEthernet::finish()
{
    if(!on)
        return;

    if(pcap_handle != NULL)
        pcap_close(pcap_handle);
}


void SniffEthernet::handleMessage(cMessage *msg)
{
    if(msg == captureEvent)
    {
        if(pcap_handle != NULL)
        {
            struct pcap_pkthdr *header;
            const u_char *pkt_data;

            // check this link! https://www.winpcap.org/docs/docs_40_2/html/group__wpcap__tut4.html
            int res = pcap_next_ex(pcap_handle, &header, &pkt_data);

            // if the packet has been read without problems
            if(res == 1)
                SniffEthernet::got_packet(header, pkt_data);
            // if the timeout set with pcap_open_live() has elapsed.
            // In this case pkt_header and pkt_data don't point to a valid packet
            else if(res == 0)
            {
                // std::cout << "Capture timeout" << endl;
            }
            // if an error occurred
            else if(res == -1)
                error("Capture error");
            // if EOF was reached reading from an offline capture
            else if(res == -2)
                error("EOF reached from an offline capture!");

            // getting statistics
            if(printStat)
            {
                static u_int old_received = 0;
                static u_int old_dropped  = 0;
                struct pcap_stat stat;

                if(pcap_stats(pcap_handle, &stat) < 0)
                    error("Error setting the mode");

                // if either changed
                if(stat.ps_recv != old_received || stat.ps_drop != old_dropped)
                    printf("\nreceived: %u, dropped: %u \n", stat.ps_recv, stat.ps_drop);

                old_received = stat.ps_recv;
                old_dropped  = stat.ps_drop;
            }
        }

        std::cout.flush();

        scheduleAt(simTime() + 0.05, captureEvent);
    }
}


void SniffEthernet::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        SniffEthernet::executeEachTimestep();
    }
    else if(signalID == Signal_executeFirstTS)
    {
        SniffEthernet::executeFirstTimeStep();
    }
}


void SniffEthernet::executeFirstTimeStep()
{
    if(!on)
        return;

    startSniffing();
}


void SniffEthernet::executeEachTimestep()
{

}


void SniffEthernet::listInterfaces()
{
    std::cout << std::endl << "List of all interfaces on this machine: " << std::endl;

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs;
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
        error("There is a problem with pcap_findalldevs: %s\n", errbuf);

    /* iterate over all devices */
    for(pcap_if_t *dev = alldevs; dev != NULL; dev = dev->next)
    {
        printf("Interface: %-15s", dev->name);

        /* check if the device captureble*/
        for (pcap_addr_t *dev_addr = dev->addresses; dev_addr != NULL; dev_addr = dev_addr->next)
        {
            if (dev_addr->addr && dev_addr->addr->sa_family == AF_INET && dev_addr->netmask)
            {
                uint32_t netAdd = ( (struct sockaddr_in *)(dev_addr->addr) )->sin_addr.s_addr;
                uint32_t netMask = ( (struct sockaddr_in *)(dev_addr->netmask) )->sin_addr.s_addr;
                printf("Address: %-16s  NetMask: %-16s", formatIPaddress(netAdd).c_str(), formatIPaddress(netMask).c_str());

                // add interface information
                devDesc *des = new devDesc(netAdd, netMask);
                allDev.insert(std::make_pair(dev->name, *des));

                break;
            }
        }

        if (dev->description)
            printf("%-40s", dev->description);
        else
            printf("%-40s", "No description available");

        printf("\n");
    }

    pcap_freealldevs(alldevs);

    std::cout.flush();
}


void SniffEthernet::getOUIFromFile()
{
    // OUI is downloaded from https://www.wireshark.org/tools/oui-lookup.html
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path manuf_FullPath = VENTOS_FullPath / "application/sniffing/sniff_manuf";

    std::ifstream in(manuf_FullPath.string().c_str());
    if(in.fail())
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


void SniffEthernet::getPortNumbersFromFile()
{
    // services files in Linux is in /etc/services
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path services_FullPath = VENTOS_FullPath / "application/sniffing/sniff_services";
    std::ifstream in(services_FullPath.string().c_str());
    if(in.fail())
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


std::string SniffEthernet::formatMACaddress(const unsigned char MACData[])
{
    boost::format fmt("%02X:%02X:%02X:%02X:%02X:%02X");

    for (int i = 0; i != 6; ++i)
        fmt % static_cast<unsigned int>(MACData[i]);

    return fmt.str();
}


// u_int8_t arp_spa[4];
std::string SniffEthernet::formatIPaddressMAC(const unsigned char addr[])
{
    boost::format fmt("%u.%u.%u.%u");

    for(int i = 0; i < 4; ++i)
        fmt % static_cast<unsigned int>(addr[i]);

    return fmt.str();
}


// u_int32_t saddr
std::string SniffEthernet::formatIPaddress(uint32_t addr)
{
    in_addr srcAdd;
    srcAdd.s_addr = addr;

    return inet_ntoa(srcAdd);

    //    Alternative implementation
    //    std::stringstream finalAdd;
    //
    //    finalAdd << (addr & 0xFF) << ".";
    //    finalAdd << (addr >> 8 & 0xFF) << ".";
    //    finalAdd << (addr >> 16 & 0xFF) << ".";
    //    finalAdd << (addr >> 24 & 0xFF);
    //
    //    return finalAdd.str();
}


std::string SniffEthernet::MACtoOUI(const unsigned char MACData[])
{
    // get the first three octet
    boost::format fmt("%02X:%02X:%02X");

    for (int i = 0; i != 3; ++i)
        fmt % static_cast<unsigned int>(MACData[i]);

    auto it = OUI.find(fmt.str());
    if(it != OUI.end())
        return it->second;
    else
        return "?";
}


std::string SniffEthernet::portToApplication(int port)
{
    auto it = portNumber.find(port);

    if(it != portNumber.end())
        return it->second;
    else return "?";
}


void SniffEthernet::startSniffing()
{
    // very useful information: http://pcap.man.potaroo.net/

    // check if interface is valid!
    auto iterfacePtr = allDev.find(interface);
    if(iterfacePtr == allDev.end())
        error("%s is not capturable!", interface.c_str());

    printf(">>> Capturing started on %s with filter \"%s\" ...\n", interface.c_str(), filter_exp.c_str());
    std::cout.flush();

    // if something happens, place error string here
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_handle = pcap_create(interface.c_str(), errbuf);
    if (pcap_handle == NULL)
        error("pcap_create failed! %s", errbuf);

    // maximum size of packets to capture in bytes
    int status = pcap_set_snaplen(pcap_handle, SNAP_LEN);
    if (status < 0)
        error("pcap_set_snaplen failed!");

    // set card in promiscuous mode?
    status = pcap_set_promisc(pcap_handle, 1);
    if (status < 0)
        error("pcap_set_promisc failed!");

    /* call the packet handler function directly when a packet is received without wait the timeout.
       In immediate mode packets get delivered to the application as soon as they
       arrive, rather than after a timeout. This makes it very important for latency
       sensitive live captures. */
    //status = pcap_set_immediate_mode(pcap_handle, 1);  // todo: not supported on Ubuntu 12.04
    //if (status < 0)
        //error("pcap_set_immediate_mode failed!");

    // time to wait for packets in miliseconds before read times out
    status = pcap_set_timeout(pcap_handle, 1);
    if (status < 0)
        error("pcap_set_timeout failed!");

    // buffer size in byte (1 MB = 1048576 B)
    status = pcap_set_buffer_size(pcap_handle, 1048576);
    if (status < 0)
        error("pcap_set_buffer_size failed!");

    pcap_activate(pcap_handle);

    /* make sure we're capturing on an Ethernet device [2] */
    if (pcap_datalink(pcap_handle) != DLT_EN10MB)
        error("%s is not an Ethernet", interface.c_str());

    /* compile the filter expression */
    struct bpf_program fp;
    if (pcap_compile(pcap_handle, &fp, filter_exp.c_str(), 0, iterfacePtr->second.netMask) == -1)
        error("Couldn't parse filter %s: %s\n", filter_exp.c_str(), pcap_geterr(pcap_handle));

    /* apply the compiled filter */
    if (pcap_setfilter(pcap_handle, &fp) == -1)
        error("Couldn't install filter %s: %s\n", filter_exp.c_str(), pcap_geterr(pcap_handle));

    pcap_freecode(&fp);

    // start capturing packets
    scheduleAt(simTime() + 0.00001, captureEvent);
}


void SniffEthernet::got_packet(const struct pcap_pkthdr *header, const u_char *packet)
{
    static int count = 1;  /* packet counter */

    /* convert the timestamp to readable format */
    time_t local_tv_sec = header->ts.tv_sec;
    struct tm *ltime = localtime(&local_tv_sec);
    char timestr[16];
    strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

    if(printCaptured) printf("\n#%-3d %s.%.6ld \n", count, timestr, header->ts.tv_usec);
    count++;

    if(printCaptured) printf("Ethernet Frame --> ");

    /* define Ethernet header */
    const struct ether_header *p = (struct ether_header*)(packet);

    /* what packet type we have..*/
    if (ntohs (p->ether_type) == ETHERTYPE_ARP)
    {
        if(printCaptured)
        {
            printf("Src MAC: %s (%s), ", formatMACaddress(p->ether_shost).c_str(), MACtoOUI(p->ether_shost).c_str());
            printf("Dst MAC: %s (%s), ", formatMACaddress(p->ether_dhost).c_str(), MACtoOUI(p->ether_dhost).c_str());
            printf("Type: ETHERTYPE_ARP \n");
        }

        processARP(packet);
    }
    else if (ntohs (p->ether_type) == ETHERTYPE_IP)
    {
        if(printCaptured)
        {
            printf("Src MAC: %s (%s), ", formatMACaddress(p->ether_shost).c_str(), MACtoOUI(p->ether_shost).c_str());
            printf("Dst MAC: %s (%s), ", formatMACaddress(p->ether_dhost).c_str(), MACtoOUI(p->ether_dhost).c_str());
            printf("Type: ETHERTYPE_IP \n");
        }

        processIPv4(packet);
    }
    else  if (ntohs (p->ether_type) == ETHERTYPE_IPV6)
    {
        if(printCaptured)
        {
            printf("Src MAC: %s (%s), ", formatMACaddress(p->ether_shost).c_str(), MACtoOUI(p->ether_shost).c_str());
            printf("Dst MAC: %s (%s), ", formatMACaddress(p->ether_dhost).c_str(), MACtoOUI(p->ether_dhost).c_str());
            printf("Type: ETHERTYPE_IPV6 \n");
        }

        processIPv6(packet);
    }
    else
    {
        printf("Invalid Ethernet type %x", ntohs(p->ether_type));
        return;
    }
}


void SniffEthernet::processARP(const u_char *packet)
{
    const struct ether_arp *arp = (struct ether_arp*)(packet + ETHER_HDR_LEN);

    if(printCaptured)
    {
        printf("    ARP message --> ");

        // print header information
        printf("HrwType: %u, ", ntohs(arp->ea_hdr.ar_hrd));
        printf("ProtocolType: 0x%02X, ", ntohs(arp->ea_hdr.ar_pro));
        printf("HrwLen: %u, ", arp->ea_hdr.ar_hln);
        printf("Protocol len: %u, ", arp->ea_hdr.ar_pln);
        printf("Opcode: %u, \n", ntohs(arp->ea_hdr.ar_op));

        // print payload information
        printf("                    ");
        printf("SenderHrwAdd: %s, ", formatMACaddress(arp->arp_sha).c_str());
        printf("SenderProtAdd: %s \n", formatIPaddressMAC(arp->arp_spa).c_str());
        printf("                    ");
        printf("TargetHrwAdd: %s, ", formatMACaddress(arp->arp_tha).c_str());
        printf("TargetProtAdd: %s", formatIPaddressMAC(arp->arp_tpa).c_str());
    }
}


void SniffEthernet::processIPv4(const u_char *packet)
{
    const struct iphdr *ip = (struct iphdr*)(packet + ETHER_HDR_LEN);

    if(ip->version != 4)
    {
        printf("    Invalid IPv4 version number: %u ", ip->version);
        return;
    }

    int size_ip = ip->ihl * 4;  // size in byte
    if (size_ip < 20) // minimum ipv4 header size is 20 bytes
    {
        printf("    Invalid IPv4 header length: %u bytes", size_ip);
        return;
    }

    if(printCaptured)
    {
        printf("    IPv4 packet --> ");

        /* print source and destination IP addresses */
        printf("From: %s, ", formatIPaddress(ip->saddr).c_str());
        printf("To: %s, ", formatIPaddress(ip->daddr).c_str());
    }

    /* determine protocol */
    switch(ip->protocol)
    {
    case IPPROTO_TCP:
        if(printCaptured) printf("Protocol: %s", "TCP");
        processTCP(packet, ip);
        break;
    case IPPROTO_UDP:
        if(printCaptured) printf("Protocol: %s", "UDP");
        processUDP(packet, ip);
        break;
    case IPPROTO_ICMP:
        if(printCaptured) printf("Protocol: %s", "ICMP");
        processICMP(packet, ip);
        break;
    default:
        if(printCaptured) printf("Protocol: %s", "?");
        break;
    }
}


void SniffEthernet::processIPv6(const u_char *packet)
{
    // const struct ip6_hdr *ipv6 = (struct ip6_hdr*)(packet + ETHER_HDR_LEN);

//    if(ip->version != 4)
//    {
//        printf("    Invalid IPv6 version number: %u ", ip->version);
//        return;
//    }


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

    if(printCaptured)
    {
        printf("\n        TCP segment --> ");

        printf("Src port: %d (%s), ", ntohs(tcp->th_sport), portToApplication(ntohs(tcp->th_sport)).c_str());
        printf("Dst port: %d (%s), ", ntohs(tcp->th_dport), portToApplication(ntohs(tcp->th_dport)).c_str());
    }

    /* compute tcp payload (segment) offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    int size_payload = ntohs(ip->tot_len) - (size_ip + size_tcp);
    if(printCaptured) printf("Payload (%d bytes)\n", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


void SniffEthernet::processUDP(const u_char *packet, const struct iphdr *ip)
{
    /* compute udp header offset */
    int size_ip = ip->ihl * 4;  // size in byte
    const struct udphdr *udp = (struct udphdr*)(packet + ETHER_HDR_LEN + size_ip);

    int size_udp = udp->uh_ulen;  // size in byte
    if (size_udp < 8)
    {
        printf("\n        Invalid UDP length: %u bytes, ", size_udp);
        return;
    }

    if(printCaptured)
    {
        printf("\n        UDP datagram --> ");

        printf("Src port: %d (%s), ", ntohs(udp->uh_sport), portToApplication(ntohs(udp->uh_sport)).c_str());
        printf("Dst port: %d (%s), ", ntohs(udp->uh_dport), portToApplication(ntohs(udp->uh_dport)).c_str());
    }

    /* compute udp payload offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + 8);

    /* compute udp payload size */
    int size_payload = ntohs(ip->tot_len) - (size_ip + 8);
    if(printCaptured) printf("Payload (%d bytes)\n", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


void SniffEthernet::processICMP(const u_char *packet, const struct iphdr *ip)
{
    /* compute icmp header offset */
    int size_ip = ip->ihl * 4;  // size in byte
    const struct icmphdr *icmp = (struct icmphdr*)(packet + ETHER_HDR_LEN + size_ip);

    if(printCaptured)
    {
        printf("\n        ICMP message --> ");

        printf("Type: %d, ", icmp->type);
        printf("Code: %d, ", icmp->code);
    }

    /* compute icmp payload offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + 8);

    /* compute icmp payload size */
    int size_payload = ntohs(ip->tot_len) - (size_ip + 8);
    if(printCaptured) printf("Payload (%d bytes)\n", size_payload);

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
