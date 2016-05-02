/****************************************************************************/
/// @file    SNMP.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2015
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

#define WANT_WINSOCK2
#include <platdep/sockets.h>

#include "SNMP.h"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {


SNMP::~SNMP()
{
    if(SNMP_session != NULL)
        SNMP_session->socket_cleanup();  // Shut down socket subsystem
}


// constructor
SNMP::SNMP(std::string host, std::string port)
{
    Snmp_pp::IpAddress IPAddress(host.c_str());

    if(!IPAddress.valid())
        throw omnetpp::cRuntimeError("IP address %s is not valid!", IPAddress.get_printable());

    std::cout << std::endl << "Pinging " << IPAddress.get_printable() << " ... ";
    std::cout.flush();

    // test if IPAdd is alive?
    std::string cmd = "ping -c 1 -s 1 " + std::string(IPAddress.get_printable()) + " > /dev/null 2>&1";
    int result = system(cmd.c_str());

    if(result != 0)
        throw omnetpp::cRuntimeError("device at %s is not responding!", IPAddress.get_printable());

    std::cout << "Done! \n";
    std::cout << "Creating SNMP session ... ";
    std::cout.flush();

    int randomPort = getFreeEphemeralPort();

    int status;   // return status
    SNMP_session = new Snmp_pp::Snmp(status, randomPort);  // create a SNMP++ session
    if (status != SNMP_CLASS_SUCCESS) // check creation status
        throw omnetpp::cRuntimeError("%s", SNMP_session->error_msg(status));  // if fail, print error string

    std::cout << "Done! -- ephemeral port " << randomPort << "\n";
    std::cout.flush();

    Snmp_pp::UdpAddress address(IPAddress);
    address.set_port(std::stoi(port));         // SNMP port for Econolite virtual controller

    ctarget = new Snmp_pp::CTarget(address);   // community target

    ctarget->set_version(Snmp_pp::version1);   // set the SNMP version to SNMPV1
    ctarget->set_retry(1);                     // set the number of auto retries
    ctarget->set_timeout(100);                 // set timeout
    Snmp_pp::OctetStr rcommunity("public");    // read community name
    ctarget->set_readcommunity(rcommunity);
    Snmp_pp::OctetStr wcommunity("private");
    ctarget->set_writecommunity(wcommunity);   // write community name

    std::cout << "Connected to " << ctarget->get_address().get_printable();
    std::cout << std::endl;

    // construct the snmp log file path
    boost::filesystem::path VENTOS_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    std::ostringstream fileName;
    fileName << "snmp_" << IPAddress.get_printable() << "_" << randomPort << ".log";
    boost::filesystem::path logFilePath = VENTOS_FullPath / "results" / fileName.str();
    std::cout << "Logging at " << logFilePath.string();
    std::cout << std::endl << std::endl;

    // set the log file
    Snmp_pp::AgentLogImpl *logFile = new Snmp_pp::AgentLogImpl(logFilePath.string().c_str());
    Snmp_pp::DefaultLog::init(logFile);

    // set filter for logging
    Snmp_pp::DefaultLog::log()->set_profile("full");
    Snmp_pp::DefaultLog::log()->set_filter(ERROR_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(WARNING_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(EVENT_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(INFO_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(DEBUG_LOG, 7);
}


// get a random Ephemeral port from the system
int SNMP::getFreeEphemeralPort()
{
    if (initsocketlibonce() != 0)
        throw omnetpp::cRuntimeError("Could not init socketlib");

    SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        throw omnetpp::cRuntimeError("Failed to create socket: %s", strerror(errno));

    struct sockaddr_in serv_addr;
    struct sockaddr* serv_addr_p = (struct sockaddr*)&serv_addr;
    memset(serv_addr_p, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = 0;   // get a random Ephemeral port

    if (::bind(sock, serv_addr_p, sizeof(serv_addr)) < 0)
        throw omnetpp::cRuntimeError("Failed to bind socket: %s", strerror(errno));

    socklen_t len = sizeof(serv_addr);
    if (getsockname(sock, serv_addr_p, &len) < 0)
        throw omnetpp::cRuntimeError("Failed to get hostname: %s", strerror(errno));

    int port = ntohs(serv_addr.sin_port);

    closesocket(sock);

    return port;
}


Snmp_pp::Vb SNMP::SNMPget(std::string OID, int instance)
{
    // make sure snmp pointer is valid
    ASSERT(SNMP_session);

    if(instance < 0)
        throw omnetpp::cRuntimeError("instance in SNMPget is less than zero!");

    // add the instance to the end of oid
    OID = OID + "." + std::to_string(instance);

    Snmp_pp::Oid myOID(OID.c_str());
    Snmp_pp::Vb vb(myOID);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = SNMP_session->get(pdu, *ctarget);   // invoke a SNMP++ get

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << SNMP_session->error_msg(status) << std::endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}


// For non-tabular objects, the given OID is queried (similar to a SNMPget)
// For tabular objects, all variables in the subtree below the given OID are queried
std::vector<Snmp_pp::Vb> SNMP::SNMPwalk(std::string OID)
{
    // make sure snmp pointer is valid
    ASSERT(SNMP_session);

    Snmp_pp::Oid myOID(OID.c_str());
    Snmp_pp::Vb vb(myOID);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = 0;
    const int BULK_MAX = 10;
    std::vector<Snmp_pp::Vb> collection;

    while( (status = SNMP_session->get_bulk(pdu, *ctarget, 0, BULK_MAX)) == SNMP_CLASS_SUCCESS )
    {
        for (int z = 0; z < pdu.get_vb_count(); z++)
        {
            pdu.get_vb(vb,z);

            Snmp_pp::Oid tmp;
            vb.get_oid(tmp);

            // End of SUBTREE Reached
            if (myOID.nCompare(myOID.len(), tmp) != 0)
                return collection;

            // look for var bind exception (End of MIB Reached), applies to v2 only
            if (vb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW)
                return collection;
            else
                collection.push_back(vb);
        }

        // last vb becomes seed of next request
        pdu.set_vblist(&vb, 1);
    }

    if (status != SNMP_ERROR_NO_SUCH_NAME)
        std::cout << "SNMP++ snmpWalk Error, " << SNMP_session->error_msg(status) << std::endl;

    return collection;
}


Snmp_pp::Vb SNMP::SNMPset(std::string OID, std::string value, int instance)
{
    // make sure snmp pointer is valid
    ASSERT(SNMP_session);

    if(instance < 0)
        throw omnetpp::cRuntimeError("instance in SNMPget is less than zero!");

    // add the instance to the end of oid
    OID = OID + "." + std::to_string(instance);

    Snmp_pp::Oid myOID(OID.c_str());

    Snmp_pp::Vb vb(myOID);   // variable Binding Object
    vb.set_value(value.c_str());

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = SNMP_session->set(pdu, *ctarget);     // invoke a SNMP++ set

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << SNMP_session->error_msg(status) << std::endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}

}

