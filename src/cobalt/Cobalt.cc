/****************************************************************************/
/// @file    Cobalt.cc
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

#include "Cobalt.h"
#include "MIB_OBJ_ASC.h"

namespace VENTOS {

Define_Module(VENTOS::Cobalt);

Cobalt::~Cobalt()
{

}


void Cobalt::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        ASSERT(module);
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        simulation.getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        on = par("on").boolValue();

        if(on)
            init_cobalt();
    }
}


void Cobalt::finish()
{
    if(cobaltSNMP)
        delete cobaltSNMP;
}


void Cobalt::handleMessage(cMessage *msg)
{

}


void Cobalt::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    if(!on)
        return;

    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimestep();
    }
}


void Cobalt::initialize_withTraCI()
{

}


void Cobalt::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (on && !wasExecuted)
    {
        cobaltSNMP->SNMPset("1.3.6.1.2.1.1.4", "manoo");

        std::vector<Snmp_pp::Vb> result = cobaltSNMP->SNMPwalk("1.3.6.1.4.1.1206.4.2.1.11.1");
        for(auto& entry : result)
        {
            std::cout << entry.get_printable_oid() << " = ";
            std::cout << entry.get_printable_value() << endl;
        }

        wasExecuted = true;
    }
}


void Cobalt::init_cobalt()
{
    std::string host = par("host").stringValue();
    std::string port = par("port").stringValue();
    cobaltSNMP = new SNMP(host, port);
    ASSERT(cobaltSNMP);

    Snmp_pp::Vb name = cobaltSNMP->SNMPget(sysName);
    std::cout << "Name: " << name.get_printable_value() << endl;

    Snmp_pp::Vb company = cobaltSNMP->SNMPget(sysContact);
    std::cout << "Company: " << company.get_printable_value() << endl;

    Snmp_pp::Vb address = cobaltSNMP->SNMPget(sysLocation);
    std::cout << "Address: " << address.get_printable_value() << endl;

    Snmp_pp::Vb upTime = cobaltSNMP->SNMPget(sysUpTime);
    std::cout << "UpTime: " << upTime.get_printable_value() << endl << endl;
}

}

