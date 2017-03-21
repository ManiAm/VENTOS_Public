/****************************************************************************/
/// @file    util.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jan 2017
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

#include <pcap.h>
#include <arpa/inet.h>
#include <fstream>

#undef ev
#include "boost/filesystem.hpp"

#include "interfacing/DB/util.h"
#include "logging/VENTOS_logging.h"

namespace VENTOS {

std::map<std::string, std::string> util::OUI;
std::map<int, std::string> util::portNumber;

util::~util()
{

}


util::util()
{

}


std::string util::MACaddrTostr(const u_int8_t MACaddr[])
{
    boost::format fmt("%02X:%02X:%02X:%02X:%02X:%02X");

    for (int i = 0; i != 6; ++i)
        fmt % static_cast<unsigned int>(MACaddr[i]);

    return fmt.str();
}


// u_int8_t arp_spa[4]
std::string util::IPaddrTostr(const u_int8_t IPaddr[])
{
    boost::format fmt("%u.%u.%u.%u");

    for(int i = 0; i < 4; ++i)
        fmt % static_cast<unsigned int>(IPaddr[i]);

    return fmt.str();
}


// u_int32_t saddr
std::string util::IPaddrTostr(uint32_t IPaddr)
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


std::string util::serverPortTostr(int port)
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


std::string util::OUITostr(const u_int8_t MACaddr[])
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

}
