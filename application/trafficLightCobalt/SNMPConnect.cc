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
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SNMP_LOG = VENTOS_FullPath / "results" / "snmp_pp.log";

        on = par("on").boolValue();

        if(on)
        {
            SNMPInitialize();

            Snmp_pp::Vb name = get(sysName);
            std::cout << "Connected to '" << name.get_printable_value() << "'" << " at " << ctarget->get_address().get_printable() << endl;

            Snmp_pp::Vb company = get(sysContact);
            std::cout << company.get_printable_value() << endl;

            Snmp_pp::Vb address = get(sysLocation);
            std::cout << address.get_printable_value() << endl;

            Snmp_pp::Vb upTime = get(sysUpTime);
            std::cout << "UpTime is " << upTime.get_printable_value() << endl;


            Snmp_pp::Vb tab = getColumn(phaseNumber_C);
            std::cout << tab.get_printable_value() << endl;
        }
    }
}


void SNMPConnect::finish()
{
    snmp->socket_cleanup();  // Shut down socket subsystem
}


void SNMPConnect::handleMessage(cMessage *msg)
{

}


void SNMPConnect::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS && on)
    {

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

    int status;                                // return status
    snmp = new Snmp_pp::Snmp(status, 0);       // create a SNMP++ session (0: bind to any port)
    if (status != SNMP_CLASS_SUCCESS)          // check creation status
        error("%s", snmp->error_msg(status));  // if fail, print error string

    std::cout << "Done!" << endl << endl;

    Snmp_pp::UdpAddress address(IPAddress);
    address.set_port(501);                     // SNMP port for econolite virtual controller

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


Snmp_pp::Vb SNMPConnect::get(std::string OID)
{
    // make sure snmp pointer is valid
    ASSERT(snmp);

    Snmp_pp::Oid SYSDESCR(OID.c_str());
    Snmp_pp::Vb vb(SYSDESCR);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = snmp->get(pdu, *ctarget);     // invoke a SNMP++ get

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << snmp->error_msg(status) << endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}


Snmp_pp::Vb SNMPConnect::getColumn(std::string OID)
{
    // make sure snmp pointer is valid
    ASSERT(snmp);

    Snmp_pp::Oid SYSDESCR(OID.c_str());
    Snmp_pp::Vb vb(SYSDESCR);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = snmp->get(pdu, *ctarget);     // invoke a SNMP++ get

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << snmp->error_msg(status) << endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}


Snmp_pp::Vb SNMPConnect::set(std::string OID)
{
    // make sure snmp pointer is valid
    ASSERT(snmp);

    Snmp_pp::Oid SYSDESCR(OID.c_str());
    Snmp_pp::Vb vb(SYSDESCR);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    int status = snmp->set(pdu, *ctarget);     // invoke a SNMP++ set

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << snmp->error_msg(status) << endl;
    else
        pdu.get_vb(vb,0);   // extract the variable binding from PDU

    return vb;
}


}

