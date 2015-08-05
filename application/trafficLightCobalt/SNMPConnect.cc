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
            SNMPInitialize();
    }
}


void SNMPConnect::finish()
{

}


void SNMPConnect::handleMessage(cMessage *msg)
{

}


void SNMPConnect::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS && on)
    {
        // ask for sysDescr
        SNMPGet(sysDescr);
    }
}


void SNMPConnect::SNMPInitialize()
{
    IPAddress = new Snmp_pp::IpAddress(par("IPAddress").stringValue());

    if(!IPAddress->valid())
        error("IP address %s is not valid!", IPAddress->get_printable());

    std::cout << endl << "Pinging " << IPAddress->get_printable() << " ... ";
    std::cout.flush();

    // test if IPAdd is alive?
    std::string cmd = "ping -c 1 -s 1 " + std::string(IPAddress->get_printable()) + " > /dev/null 2>&1";
    int result = system(cmd.c_str());

    if(result != 0)
        error("device at %s is not responding!", IPAddress->get_printable());

    std::cout << "Done!" << endl;
    std::cout << "Creating SNMP session ... ";

    int status;                                // return status
    snmp = new Snmp_pp::Snmp(status);          // create a SNMP++ session
    if (status != SNMP_CLASS_SUCCESS)          // check creation status
        error("%s", snmp->error_msg(status));  // if fail, print error string

    std::cout << "Done!" << endl;

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


void SNMPConnect::SNMPGet(std::string OID)
{
    // make sure snmp pointer is valid
    ASSERT(snmp);

    Snmp_pp::Oid SYSDESCR(OID.c_str());
    Snmp_pp::Vb vb(SYSDESCR);   // variable Binding Object

    Snmp_pp::Pdu pdu;
    pdu += vb;    // add the variable binding to the PDU

    Snmp_pp::CTarget ctarget(*IPAddress);     // community target

    ctarget.set_version(Snmp_pp::version1);   // set the SNMP version SNMPV1
    ctarget.set_retry(1);                     // set the number of auto retries
    ctarget.set_timeout(100);                 // set timeout
    Snmp_pp::OctetStr community("public");    // community name
    ctarget.set_readcommunity(community);     // set the read community name

    int status = snmp->get(pdu, ctarget);     // invoke a SNMP++ Get

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << snmp->error_msg(status);
    else
    {
        pdu.get_vb(vb,0);   // extract the variable binding from PDU
        std::cout << "System Descriptor = " << vb.get_printable_value();  // print out the value
    }
}


}

