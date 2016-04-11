/****************************************************************************/
/// @file    SNMPConnect.h
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

#ifndef SNMPCONNECT_H_
#define SNMPCONNECT_H_

#include <BaseApplLayer.h>
#include "TraCICommands.h"
#include "MIB_OBJ_ASC.h"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

#include <sys/socket.h>        // solve in Mac os x: unknown type name socklen_t
#define STDCXX_98_HEADERS
#include "snmp_pp/snmp_pp.h"

namespace VENTOS {

class SNMPConnect : public BaseApplLayer
{
public:
    virtual ~SNMPConnect();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

    Snmp_pp::Vb SNMPget(std::string OID, int instance=0);
    std::vector<Snmp_pp::Vb> SNMPwalk(std::string OID);
    template <typename T> Snmp_pp::Vb SNMPset(std::string OID, T value, int instance=0);

private:
    void SNMPInitialize();

private:
    typedef BaseApplLayer super;

    // NED variables
    TraCI_Commands *TraCI;  // pointer to the TraCI module
    simsignal_t Signal_executeFirstTS;
    bool on;

    boost::filesystem::path SNMP_LOG;
    Snmp_pp::Snmp *cobalt = NULL;
    Snmp_pp::CTarget *ctarget = NULL;
};

}

#endif
