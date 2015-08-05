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

#include <libsnmp.h>
#include "snmp_pp/snmp_pp.h"

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

        on = par("on").boolValue();
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

    if(signalID == Signal_executeFirstTS)
    {
        SNMP_example();
    }
}


void SNMPConnect::SNMP_example()
{
    int status;                         // return status
    Snmp_pp::Snmp snmp(status);         // create a SNMP++ session
    if (status != SNMP_CLASS_SUCCESS)   // check creation status
    {
        std::cout << snmp.error_msg(status);  // if fail, print error string
        return;
    }

    Snmp_pp::Oid SYSDESCR("1.3.6.1.2.1.1.1.0");   // Object ID for System Descriptor
    Snmp_pp::Vb vb(SYSDESCR);                     // SNMP++ Variable Binding Object

    Snmp_pp::Pdu pdu;                             // SNMP++ PDU
    pdu += vb;                                    // add the variable binding to the PDU

    Snmp_pp::IpAddress address("localhost");
    Snmp_pp::CTarget ctarget(address);            // SNMP++ community target

    status = snmp.get(pdu, ctarget);              // Invoke a SNMP++ Get

    if (status != SNMP_CLASS_SUCCESS)
        std::cout << snmp.error_msg(status);
    else
    {
        pdu.get_vb(vb,0);     // extract the variable binding from PDU
        std::cout << "System Descriptor = " << vb.get_printable_value();  // print out the value
    }
}


}

