/****************************************************************************/
/// @file    SNMPConnect.cc
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

#include "SNMPConnect.h"

namespace VENTOS {

Define_Module(VENTOS::SNMPConnect);


SNMPConnect::~SNMPConnect()
{

}


void SNMPConnect::initialize(int stage)
{
    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SNMP_LOG = VENTOS_FullPath / "results" / "snmp_pp.log";

        on = par("on").boolValue();

        if(on)
        {
            SNMPInitialize();

            Snmp_pp::Vb name = SNMPget(sysName);
            std::cout << "Connected to '" << name.get_printable_value() << "'" << " at " << ctarget->get_address().get_printable() << endl;

            Snmp_pp::Vb company = SNMPget(sysContact);
            std::cout << company.get_printable_value() << endl;

            Snmp_pp::Vb address = SNMPget(sysLocation);
            std::cout << address.get_printable_value() << endl;

            Snmp_pp::Vb upTime = SNMPget(sysUpTime);
            std::cout << "UpTime is " << upTime.get_printable_value() << endl << endl;
        }
    }
}


void SNMPConnect::finish()
{
    cobalt->socket_cleanup();  // Shut down socket subsystem
}


void SNMPConnect::handleMessage(cMessage *msg)
{

}


void SNMPConnect::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS && on)
    {
        // todo:

        SNMPset("1.3.6.1.2.1.1.4", "manoo");


//        std::vector<Snmp_pp::Vb> result = SNMPwalk("1.3.6.1.4.1.1206.4.2.1.11.1");
//        for(auto& entry : result)
//        {
//            std::cout << entry.get_printable_oid() << " = ";
//            std::cout << entry.get_printable_value() << endl;
//        }
    }
}


void SNMPConnect::SNMPInitialize()
{
    Snmp_pp::IpAddress IPAddress(par("IPAddress").stringValue());

    if(!IPAddress.valid())
        error("IP address %s is not valid!", IPAddress.get_printable());

    std::cout << endl << "Pinging " << IPAddress.get_printable() << " ... ";
    std::cout.flush();

    // test if IPAdd is alive?
    std::string cmd = "ping -c 1 -s 1 " + std::string(IPAddress.get_printable()) + " > /dev/null 2>&1";
    int result = system(cmd.c_str());

    if(result != 0)
        error("device at %s is not responding!", IPAddress.get_printable());

    std::cout << "Done!" << endl;
    std::cout << "Creating SNMP session ... ";

    int status;                                  // return status
    cobalt = new Snmp_pp::Snmp(status, 0);       // create a SNMP++ session (0: bind to any port)
    if (status != SNMP_CLASS_SUCCESS)            // check creation status
        error("%s", cobalt->error_msg(status));  // if fail, print error string

    std::cout << "Done!" << endl << endl;

    Snmp_pp::UdpAddress address(IPAddress);
    std::string port = par("port").stringValue();
    address.set_port( std::stoi(port) );       // SNMP port for econolite virtual controller

    ctarget = new Snmp_pp::CTarget(address);   // community target

    ctarget->set_version(Snmp_pp::version1);   // set the SNMP version SNMPV1
    ctarget->set_retry(1);                     // set the number of auto retries
    ctarget->set_timeout(100);                 // set timeout
    Snmp_pp::OctetStr rcommunity("public");    // read community name
    ctarget->set_readcommunity(rcommunity);
    Snmp_pp::OctetStr wcommunity("private");
    ctarget->set_writecommunity(wcommunity);   // write community name

    // set the log file
    Snmp_pp::AgentLogImpl *logFile = new Snmp_pp::AgentLogImpl(SNMP_LOG.string().c_str());
    Snmp_pp::DefaultLog::init(logFile);

    // set filter for logging
    Snmp_pp::DefaultLog::log()->set_profile("full");
    Snmp_pp::DefaultLog::log()->set_filter(ERROR_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(WARNING_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(EVENT_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(INFO_LOG, 7);
    Snmp_pp::DefaultLog::log()->set_filter(DEBUG_LOG, 7);
}


Snmp_pp::Vb SNMPConnect::SNMPget(std::string OID, int instance)
{
    // make sure snmp pointer is valid
    ASSERT(cobalt);

    if(instance < 0)
        error("instance in SNMPget is less than zero!");

    // add the instance to the end of oid
    OID = OID + "." + std::to_string(instance);

    Snmp_pp::Oid myOID(OID.c_str());
    Snmp_pp::Vb vb(myOID);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = cobalt->get(pdu, *ctarget);     // invoke a SNMP++ get

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << cobalt->error_msg(status) << endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}


// For non-tabular objects, the given OID is queried (similar to a SNMPget)
// For tabular objects, all variables in the subtree below the given OID are queried
std::vector<Snmp_pp::Vb> SNMPConnect::SNMPwalk(std::string OID)
{
    // make sure snmp pointer is valid
    ASSERT(cobalt);

    Snmp_pp::Oid myOID(OID.c_str());
    Snmp_pp::Vb vb(myOID);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = 0;
    const int BULK_MAX = 10;
    std::vector<Snmp_pp::Vb> collection;

    while( (status = cobalt->get_bulk(pdu, *ctarget, 0, BULK_MAX)) == SNMP_CLASS_SUCCESS )
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
        std::cout << "SNMP++ snmpWalk Error, " << cobalt->error_msg(status) << endl;

    return collection;
}


template <typename T>
Snmp_pp::Vb SNMPConnect::SNMPset(std::string OID, T value, int instance)
{
    // make sure snmp pointer is valid
    ASSERT(cobalt);

    if(instance < 0)
        error("instance in SNMPget is less than zero!");

    // add the instance to the end of oid
    OID = OID + "." + std::to_string(instance);

    Snmp_pp::Oid myOID(OID.c_str());

    Snmp_pp::Vb vb(myOID);   // variable Binding Object
    vb.set_value(value);

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = cobalt->set(pdu, *ctarget);     // invoke a SNMP++ set

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << cobalt->error_msg(status) << endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}


}

