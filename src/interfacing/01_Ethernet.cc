/****************************************************************************/
/// @file    Ethernet.cc
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

#include <01_Ethernet.h>
#include <fstream>
#include <thread>
#include <chrono>
#include "boost/format.hpp"

#undef ev
#include "boost/filesystem.hpp"

namespace VENTOS {

Define_Module(VENTOS::Ethernet);

Ethernet::~Ethernet()
{

}


void Ethernet::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        on = par("on").boolValue();
        if(!on)
            return;

        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        interface = par("interface").stdstringValue();
        filter_exp = par("filter_exp").stdstringValue();
        printCaptured = par("printCaptured").boolValue();
        printDataPayload = par("printDataPayload").boolValue();
        printStat = par("printStat").boolValue();

        // register signals
        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);

        listInterfaces();
    }
}


void Ethernet::finish()
{
    if(!on)
        return;

    if(pcap_handle != NULL)
        pcap_close(pcap_handle);

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void Ethernet::handleMessage(omnetpp::cMessage *msg)
{

}


void Ethernet::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        Ethernet::executeEachTimestep();
    }
    else if(signalID == Signal_initialize_withTraCI)
    {
        Ethernet::initialize_withTraCI();
    }
}


void Ethernet::initialize_withTraCI()
{

}


void Ethernet::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (on && !wasExecuted)
    {
        initSniffing();

        // launch a thread to do the sniffing
        std::thread t1(&Ethernet::startSniffing, this);
        t1.detach();

        wasExecuted = true;
    }

    {
        std::lock_guard<std::mutex> lock(theLock);

        if(!framesQueue.empty())
        {
            auto &frame = framesQueue.back();
            got_packet(frame.first, frame.second);
            framesQueue.pop_back();
        }
    }
}


void Ethernet::listInterfaces()
{
    std::cout << std::endl << ">>> List of all interfaces on this machine: " << std::endl;

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs;
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
        throw omnetpp::cRuntimeError("There is a problem with pcap_findalldevs: %s\n", errbuf);

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
                printf("Address: %-16s  NetMask: %-16s", IPaddrTostr(netAdd).c_str(), IPaddrTostr(netMask).c_str());

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

    printf("\n");

    pcap_freealldevs(alldevs);

    std::cout.flush();
}


std::string Ethernet::MACaddrTostr(const u_int8_t MACaddr[])
{
    boost::format fmt("%02X:%02X:%02X:%02X:%02X:%02X");

    for (int i = 0; i != 6; ++i)
        fmt % static_cast<unsigned int>(MACaddr[i]);

    return fmt.str();
}


// u_int8_t arp_spa[4];
std::string Ethernet::IPaddrTostr(const u_int8_t IPaddr[])
{
    boost::format fmt("%u.%u.%u.%u");

    for(int i = 0; i < 4; ++i)
        fmt % static_cast<unsigned int>(IPaddr[i]);

    return fmt.str();
}


// u_int32_t saddr
std::string Ethernet::IPaddrTostr(uint32_t IPaddr)
{
    in_addr srcAdd;
    srcAdd.s_addr = IPaddr;

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


std::string Ethernet::serverPortTostr(int port)
{
    if(portNumber.empty())
    {
        // services files in Linux is in /etc/services
        boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        boost::filesystem::path services_FullPath = VENTOS_FullPath / "src/interfacing/DB/ServerPorts";
        std::ifstream in(services_FullPath.string().c_str());
        if(in.fail())
            throw omnetpp::cRuntimeError("cannot open file sniff_services at %s", services_FullPath.string().c_str());

        std::string line;
        while(getline(in, line))
        {
            std::vector<std::string> lineToken = omnetpp::cStringTokenizer(line.c_str()).asVector();

            if(lineToken.size() < 2)
                continue;

            std::vector<std::string> port_prot = omnetpp::cStringTokenizer(lineToken[1].c_str(), "/").asVector();

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

    auto it = portNumber.find(port);

    if(it != portNumber.end())
        return it->second;
    else return "?";
}


std::string Ethernet::OUITostr(const u_int8_t MACaddr[])
{
    if(OUI.empty())
    {
        // OUI is downloaded from https://www.wireshark.org/tools/oui-lookup.html
        boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        boost::filesystem::path manuf_FullPath = VENTOS_FullPath / "src/interfacing/DB/OUI";

        std::ifstream in(manuf_FullPath.string().c_str());
        if(in.fail())
            throw omnetpp::cRuntimeError("cannot open file sniff_manuf at %s", manuf_FullPath.string().c_str());

        std::string line;
        while(getline(in, line))
        {
            std::vector<std::string> lineToken = omnetpp::cStringTokenizer(line.c_str()).asVector();

            if(lineToken.size() < 2)
                continue;

            auto it = OUI.find(lineToken[0]);
            if(it == OUI.end())
                OUI[lineToken[0]] = lineToken[1];
        }
    }

    // get the first three octet
    boost::format fmt("%02X:%02X:%02X");

    for (int i = 0; i != 3; ++i)
        fmt % static_cast<unsigned int>(MACaddr[i]);

    auto it = OUI.find(fmt.str());
    if(it != OUI.end())
        return it->second;
    else
        return "?";
}


// very useful information: http://pcap.man.potaroo.net/
void Ethernet::initSniffing()
{
    // check if interface is valid!
    auto iterfacePtr = allDev.find(interface);
    if(iterfacePtr == allDev.end())
        throw omnetpp::cRuntimeError("%s is not capturable!", interface.c_str());

    printf(">>> Capturing started on %s with filter \"%s\" ...\n", interface.c_str(), filter_exp.c_str());
    std::cout.flush();

    // if something happens, place error string here
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_handle = pcap_create(interface.c_str(), errbuf);
    if (pcap_handle == NULL)
        throw omnetpp::cRuntimeError("pcap_create failed! %s", errbuf);

    // maximum size of packets to capture in bytes
    int status = pcap_set_snaplen(pcap_handle, SNAP_LEN);
    if (status < 0)
        throw omnetpp::cRuntimeError("pcap_set_snaplen failed!");

    // set card in promiscuous mode?
    status = pcap_set_promisc(pcap_handle, 1);
    if (status < 0)
        throw omnetpp::cRuntimeError("pcap_set_promisc failed!");

    /* call the packet handler function directly when a packet is received without wait the timeout.
       In immediate mode packets get delivered to the application as soon as they
       arrive, rather than after a timeout. This makes it very important for latency
       sensitive live captures. */
    status = pcap_set_immediate_mode(pcap_handle, 1);  // todo: not supported on Ubuntu 12.04
    if (status < 0)
        throw omnetpp::cRuntimeError("pcap_set_immediate_mode failed!");

    // time to wait for packets in milliseconds before read times out -- ignored if immediate mode is set
    // check this link: http://www.tcpdump.org/manpages/pcap_set_timeout.3pcap.txt
    status = pcap_set_timeout(pcap_handle, 10000);
    if (status < 0)
        throw omnetpp::cRuntimeError("pcap_set_timeout failed!");

    // buffer size in byte (1 MB = 1048576 B)
    status = pcap_set_buffer_size(pcap_handle, 1048576);
    if (status < 0)
        throw omnetpp::cRuntimeError("pcap_set_buffer_size failed!");

    pcap_activate(pcap_handle);

    /* make sure we're capturing on an Ethernet device [2] */
    if (pcap_datalink(pcap_handle) != DLT_EN10MB)
        throw omnetpp::cRuntimeError("%s is not an Ethernet", interface.c_str());

    /* compile the filter expression */
    struct bpf_program fp;
    if (pcap_compile(pcap_handle, &fp, filter_exp.c_str(), 0, iterfacePtr->second.netMask) == -1)
        throw omnetpp::cRuntimeError("Couldn't parse filter %s: %s\n", filter_exp.c_str(), pcap_geterr(pcap_handle));

    /* apply the compiled filter */
    if (pcap_setfilter(pcap_handle, &fp) == -1)
        throw omnetpp::cRuntimeError("Couldn't install filter %s: %s\n", filter_exp.c_str(), pcap_geterr(pcap_handle));

    pcap_freecode(&fp);
}


void Ethernet::startSniffing()
{
    if(pcap_handle == NULL)
        throw omnetpp::cRuntimeError("pcap_handle is invalid!");

    struct pcap_pkthdr *header;
    const u_char *pkt_data;
    u_long timeOutCount = 0;

    while(true)
    {
        // check this link! https://www.winpcap.org/docs/docs_40_2/html/group__wpcap__tut4.html
        int res = pcap_next_ex(pcap_handle, &header, &pkt_data);

        // if an error occurred
        if(res < 0)
            throw omnetpp::cRuntimeError("Error reading the packets: %s \n", pcap_geterr(pcap_handle));
        // if the timeout set with pcap_open_live() has elapsed.
        // In this case pkt_header and pkt_data don't point to a valid packet
        else if(res == 0)
        {
            timeOutCount++;
        }
        else if(res == 1)
        {
            std::lock_guard<std::mutex> lock(theLock);
            framesQueue.push_back( std::make_pair(header,pkt_data) );
        }

        // getting statistics
        if(printStat)
        {
            static u_int old_received = 0;
            static u_int old_dropped  = 0;
            static u_long old_timeOut = 0;
            static u_int old_buffSize  = 0;
            struct pcap_stat stat;

            if(pcap_stats(pcap_handle, &stat) < 0)
                throw omnetpp::cRuntimeError("Error setting the mode");

            // if either changed
            if(stat.ps_recv != old_received || stat.ps_drop != old_dropped || old_timeOut != timeOutCount || framesQueue.size() != old_buffSize)
            {
                printf("\n");
                printf("received: %u, dropped: %u, timeOut: %lu, in buffer: %lu \n", stat.ps_recv, stat.ps_drop, timeOutCount, framesQueue.size());
                std::cout.flush();
            }

            old_received = stat.ps_recv;
            old_dropped  = stat.ps_drop;
            old_timeOut = timeOutCount;
            old_buffSize = framesQueue.size();
        }

        // sleep the child thread
        std::this_thread::sleep_for (std::chrono::milliseconds(50));
    }
}


void Ethernet::got_packet(const struct pcap_pkthdr *header, const u_char *packet)
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
            printf("Src MAC: %s (%s), ", MACaddrTostr(p->ether_shost).c_str(), OUITostr(p->ether_shost).c_str());
            printf("Dst MAC: %s (%s), ", MACaddrTostr(p->ether_dhost).c_str(), OUITostr(p->ether_dhost).c_str());
            printf("Type: ETHERTYPE_ARP \n");
        }

        processARP(packet);
    }
    else if (ntohs (p->ether_type) == ETHERTYPE_IP)
    {
        if(printCaptured)
        {
            printf("Src MAC: %s (%s), ", MACaddrTostr(p->ether_shost).c_str(), OUITostr(p->ether_shost).c_str());
            printf("Dst MAC: %s (%s), ", MACaddrTostr(p->ether_dhost).c_str(), OUITostr(p->ether_dhost).c_str());
            printf("Type: ETHERTYPE_IP \n");
        }

        processIPv4(packet);
    }
    else  if (ntohs (p->ether_type) == ETHERTYPE_IPV6)
    {
        if(printCaptured)
        {
            printf("Src MAC: %s (%s), ", MACaddrTostr(p->ether_shost).c_str(), OUITostr(p->ether_shost).c_str());
            printf("Dst MAC: %s (%s), ", MACaddrTostr(p->ether_dhost).c_str(), OUITostr(p->ether_dhost).c_str());
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


void Ethernet::processARP(const u_char *packet)
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
        printf("SenderHrwAdd: %s, ", MACaddrTostr(arp->arp_sha).c_str());
        printf("SenderProtAdd: %s \n", IPaddrTostr(arp->arp_spa).c_str());
        printf("                    ");
        printf("TargetHrwAdd: %s, ", MACaddrTostr(arp->arp_tha).c_str());
        printf("TargetProtAdd: %s", IPaddrTostr(arp->arp_tpa).c_str());
    }
}


void Ethernet::processIPv4(const u_char *packet)
{
    const struct ip *ip_packet = (struct ip*)(packet + ETHER_HDR_LEN);

    if(ip_packet->ip_v != 4)
    {
        printf("    Invalid IPv4 version number: %u ", ip_packet->ip_v);
        return;
    }

    int size_ip = ip_packet->ip_hl * 4;  // size in byte
    if (size_ip < 20) // minimum ipv4 header size is 20 bytes
    {
        printf("    Invalid IPv4 header length: %u bytes", size_ip);
        return;
    }

    if(printCaptured)
    {
        printf("    IPv4 packet --> ");

        /* print source and destination IP addresses */
        uint32_t sAdd = ( (struct in_addr)(ip_packet->ip_src) ).s_addr;
        uint32_t dAdd = ( (struct in_addr)(ip_packet->ip_dst) ).s_addr;
        printf("From: %s, ", IPaddrTostr(sAdd).c_str());
        printf("To: %s, ", IPaddrTostr(dAdd).c_str());
    }

    /* determine protocol */
    switch(ip_packet->ip_p)
    {
    case IPPROTO_TCP:
        if(printCaptured) printf("Protocol: %s", "TCP");
        processTCP(packet, ip_packet);
        break;
    case IPPROTO_UDP:
        if(printCaptured) printf("Protocol: %s", "UDP");
        processUDP(packet, ip_packet);
        break;
    case IPPROTO_ICMP:
        if(printCaptured) printf("Protocol: %s", "ICMP");
        processICMP(packet, ip_packet);
        break;
    default:
        if(printCaptured) printf("Protocol: %s", "?");
        break;
    }
}


void Ethernet::processIPv6(const u_char *packet)
{
    // const struct ip6_hdr *ipv6 = (struct ip6_hdr*)(packet + ETHER_HDR_LEN);

    //    if(ip_packet->version != 4)
    //    {
    //        printf("    Invalid IPv6 version number: %u ", ip_packet->version);
    //        return;
    //    }


    // IPv6 header is always 40 bytes long
}


void Ethernet::processTCP(const u_char *packet, const struct ip *ip_packet)
{
    /* compute tcp header offset */
    int size_ip = ip_packet->ip_hl * 4;  // size in byte
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

        printf("Src port: %d (%s), ", ntohs(tcp->th_sport), serverPortTostr(ntohs(tcp->th_sport)).c_str());
        printf("Dst port: %d (%s), ", ntohs(tcp->th_dport), serverPortTostr(ntohs(tcp->th_dport)).c_str());
    }

    /* compute tcp payload (segment) offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    int size_payload = ntohs(ip_packet->ip_len) - (size_ip + size_tcp);
    if(printCaptured) printf("Payload (%d bytes)\n", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


void Ethernet::processUDP(const u_char *packet, const struct ip *ip_packet)
{
    /* compute udp header offset */
    int size_ip = ip_packet->ip_hl * 4;  // size in byte
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

        printf("Src port: %d (%s), ", ntohs(udp->uh_sport), serverPortTostr(ntohs(udp->uh_sport)).c_str());
        printf("Dst port: %d (%s), ", ntohs(udp->uh_dport), serverPortTostr(ntohs(udp->uh_dport)).c_str());
    }

    /* compute udp payload offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + 8);

    /* compute udp payload size */
    int size_payload = ntohs(ip_packet->ip_len) - (size_ip + 8);
    if(printCaptured) printf("Payload (%d bytes)\n", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


void Ethernet::processICMP(const u_char *packet, const struct ip *ip_packet)
{
    /* compute icmp header offset */
    int size_ip = ip_packet->ip_hl * 4;  // size in byte
    const struct icmp *icmp_msg = (struct icmp*)(packet + ETHER_HDR_LEN + size_ip);

    if(printCaptured)
    {
        printf("\n        ICMP message --> ");

        printf("Type: %d, ", icmp_msg->icmp_type);
        printf("Code: %d, ", icmp_msg->icmp_code);
    }

    /* compute icmp payload offset */
    const u_char *payload = (const u_char *)(packet + ETHER_HDR_LEN + size_ip + 8);

    /* compute icmp payload size */
    int size_payload = ntohs(ip_packet->ip_len) - (size_ip + 8);
    if(printCaptured) printf("Payload (%d bytes)\n", size_payload);

    /* Print payload data; it might be binary, so don't just treat it as a string. */
    if (size_payload > 0 && printDataPayload)
        print_dataPayload(payload, size_payload);
}


/* print packet payload data (avoid printing binary data) */
void Ethernet::print_dataPayload(const u_char *payload, int len)
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
void Ethernet::print_hex_ascii_line(const u_char *payload, int len, int offset)
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
